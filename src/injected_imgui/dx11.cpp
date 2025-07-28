#include "pch.h"
#include "injected_imgui/dx11.h"
#include "injected_imgui/hook.h"
#include "injected_imgui/internal.h"

#include "gui.h"

using namespace injected_imgui::internal;

namespace injected_imgui::dx11 {

namespace {

ID3D11DeviceContext* context{};
ID3D11Device* device{};
ID3D11RenderTargetView* main_render_view = nullptr;

bool initalized = false;

/**
 * @brief Creates the main render view.
 *
 * @param swap_chain The in use swap chain.
 * @return True if the render view was successfull created.
 */
bool create_main_render_view(IDXGISwapChain* swap_chain) {
    ID3D11Texture2D* back_buffer{};
    if (auto ret =
            swap_chain->GetBuffer(0, IID_ID3D11Texture2D, reinterpret_cast<void**>(&back_buffer))
            != S_OK) {
        LOG(ERROR, "DX11 hook initalization failed: Couldn't get texture buffer ({})", ret);
        return false;
    }
    const RaiiLambda raii{[&back_buffer]() {
        if (back_buffer != nullptr) {
            back_buffer->Release();
            back_buffer = nullptr;
        }
    }};

    if (auto ret =
            device->CreateRenderTargetView(back_buffer, nullptr, &main_render_view) != S_OK) {
        LOG(ERROR, "DX11 hook initalization failed: Couldn't create render target ({})", ret);
        return false;
    }

    return true;
}

/**
 * @brief Initalizes the dx11 hook if it isn't already.
 *
 * @param swap_chain The in use swap chain.
 * @return True if the hook was successfully initalized, false otherwise.
 */
bool ensure_initalized(IDXGISwapChain* swap_chain) {
    static bool run_once = false;
    if (run_once) {
        return initalized;
    }
    run_once = true;

    DXGI_SWAP_CHAIN_DESC desc;
    auto ret = swap_chain->GetDesc(&desc);
    if (ret != S_OK) {
        LOG(ERROR, "DX11 hook initalization failed: Couldn't get swap chain descriptor ({})", ret);
        return false;
    }

    ret = swap_chain->GetDevice(IID_ID3D11Device, reinterpret_cast<void**>(&device));
    if (ret != S_OK) {
        LOG(ERROR, "DX11 hook initalization failed: Couldn't get device ({})", ret);
        return false;
    }

    // Returns void, no error checking needed
    device->GetImmediateContext(&context);

    // This function logs it's own errors
    if (!create_main_render_view(swap_chain)) {
        return false;
    }

    if (!init_win32_backend(desc.OutputWindow)) {
        return false;
    }
    if (!ImGui_ImplDX11_Init(device, context)) {
        LOG(ERROR, "DX11 hook initalization failed: ImGui DX11 init failed");
        return false;
    }

    initalized = true;
    return true;
}

}  // namespace

namespace {

// NOLINTNEXTLINE(modernize-use-using)
typedef HRESULT(INJECTED_IMGUI_STDCALL* swap_chain_present_func)(IDXGISwapChain* self,
                                                                 UINT sync_interval,
                                                                 UINT flags);
swap_chain_present_func original_swap_chain_present;

const constexpr auto SWAP_CHAIN_PRESENT_VF_INDEX = 8;
const constexpr std::string_view SWAP_CHAIN_PRESENT_NAME = "IDXGISwapChain::Present";

/**
 * @brief Hook for `IDXGISwapChain::Present`, used to inject imgui.
 */
HRESULT INJECTED_IMGUI_STDCALL swap_chain_present_hook(IDXGISwapChain* self,
                                                       UINT sync_interval,
                                                       UINT flags) {
    try {
        static bool nested_call_guard = false;
        if (nested_call_guard) {
            return original_swap_chain_present(self, sync_interval, flags);
        }
        nested_call_guard = true;
        const RaiiLambda raii{[]() { nested_call_guard = false; }};

        if (ensure_initalized(self)) {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            live_object_explorer::gui::render();

            ImGui::EndFrame();
            ImGui::Render();

            context->OMSetRenderTargets(1, &main_render_view, nullptr);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "Exception occured during DX11 render loop: {}", ex.what());
    } catch (...) {
        LOG(ERROR, "Unknown exception occured during DX11 render loop");
    }

    return original_swap_chain_present(self, sync_interval, flags);
}

}  // namespace

namespace {

// NOLINTNEXTLINE(modernize-use-using)
typedef HRESULT(INJECTED_IMGUI_STDCALL* swap_chain_resize_buffers_func)(IDXGISwapChain* self,
                                                                        UINT buffer_count,
                                                                        UINT width,
                                                                        UINT height,
                                                                        DXGI_FORMAT new_format,
                                                                        UINT swap_chain_flags);
swap_chain_resize_buffers_func original_swap_chain_resize_buffers;

const constexpr auto SWAP_CHAIN_RESIZE_BUFFERS_VF_INDEX = 13;
const constexpr std::string_view SWAP_CHAIN_RESIZE_BUFFERS_NAME = "IDXGISwapChain::ResizeBuffers";

/**
 * @brief Hook for `IDXGISwapChain::ResizeBuffers`, used to handle resizing.
 */
HRESULT INJECTED_IMGUI_STDCALL swap_chain_resize_buffers_hook(IDXGISwapChain* self,
                                                              UINT buffer_count,
                                                              UINT width,
                                                              UINT height,
                                                              DXGI_FORMAT new_format,
                                                              UINT swap_chain_flags) {
    if (initalized) {
        if (main_render_view != nullptr) {
            context->OMSetRenderTargets(0, nullptr, nullptr);
            main_render_view->Release();
        }

        ImGui_ImplDX11_InvalidateDeviceObjects();
    }

    auto ret = original_swap_chain_resize_buffers(self, buffer_count, width, height, new_format,
                                                  swap_chain_flags);

    if (initalized) {
        create_main_render_view(self);
        context->OMSetRenderTargets(1, &main_render_view, nullptr);

        D3D11_VIEWPORT view;
        view.Width = (float)width;
        view.Height = (float)height;
        view.MinDepth = 0.0;
        view.MaxDepth = 1.0;
        view.TopLeftX = 0;
        view.TopLeftY = 0;
        context->RSSetViewports(1, &view);
    }

    return ret;
}

}  // namespace

// =================================================================================================

bool hook(void) {
    const WNDCLASSEX window_class = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = DefWindowProcA,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = GetModuleHandleA(nullptr),
        .hIcon = nullptr,
        .hCursor = nullptr,
        .hbrBackground = nullptr,
        .lpszMenuName = nullptr,
        .lpszClassName = "InjectedImguiHook",
        .hIconSm = nullptr,
    };

    RegisterClassExA(&window_class);
    const RaiiLambda raii1{[&window_class]() {
        UnregisterClassA(window_class.lpszClassName, window_class.hInstance);
    }};

    HWND window =
        CreateWindowA(window_class.lpszClassName, "Injected Imgui Temp Window", WS_OVERLAPPEDWINDOW,
                      0, 0, 100, 100, nullptr, nullptr, window_class.hInstance, nullptr);
    const RaiiLambda raii2{[&window]() {
        if (window != nullptr) {
            DestroyWindow(window);
        }
    }};

    HMODULE d3d11_module = GetModuleHandleA("d3d11.dll");
    if (d3d11_module == nullptr) {
        LOG(ERROR, "DX11 hook initalization failed: Couldn't find d3d11.dll");
        return false;
    }

    auto d3d11_create_device_and_swap_chain =
        reinterpret_cast<decltype(D3D11CreateDeviceAndSwapChain)*>(
            GetProcAddress(d3d11_module, "D3D11CreateDeviceAndSwapChain"));
    if (d3d11_create_device_and_swap_chain == nullptr) {
        LOG(ERROR, "DX11 hook initalization failed: Couldn't find D3D11CreateDeviceAndSwapChain");
        return false;
    }

    const D3D_FEATURE_LEVEL feature_levels[] = {D3D_FEATURE_LEVEL_11_0};
    const DXGI_SWAP_CHAIN_DESC swap_chain_desc = {
        .BufferDesc =
            {
                .Width = 100,
                .Height = 100,
                .RefreshRate =
                    {
                        .Numerator = 60,
                        .Denominator = 1,
                    },
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
                .Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
            },
        .SampleDesc = {.Count = 1, .Quality = 0},
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 1,
        .OutputWindow = window,
        .Windowed = 1,
        .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
        .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
    };

    IDXGISwapChain* swap_chain = nullptr;
    if (d3d11_create_device_and_swap_chain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &feature_levels[0],
            sizeof(feature_levels) / sizeof(feature_levels[0]), D3D11_SDK_VERSION, &swap_chain_desc,
            &swap_chain, nullptr, nullptr, nullptr)
        != S_OK) {
        LOG(ERROR, "DX11 hook initalization failed: Couldn't create swap chain");
        return false;
    }
    const RaiiLambda raii3{[&swap_chain]() {
        if (swap_chain != nullptr) {
            swap_chain->Release();
            swap_chain = nullptr;
        }
    }};

    uintptr_t* swap_chain_vftable = *reinterpret_cast<uintptr_t**>(swap_chain);
    unrealsdk::memory::detour(swap_chain_vftable[SWAP_CHAIN_PRESENT_VF_INDEX],
                              &swap_chain_present_hook, &original_swap_chain_present,
                              SWAP_CHAIN_PRESENT_NAME);
    unrealsdk::memory::detour(swap_chain_vftable[SWAP_CHAIN_RESIZE_BUFFERS_VF_INDEX],
                              &swap_chain_resize_buffers_hook, &original_swap_chain_resize_buffers,
                              SWAP_CHAIN_RESIZE_BUFFERS_NAME);

    return true;
}

}  // namespace injected_imgui::dx11
