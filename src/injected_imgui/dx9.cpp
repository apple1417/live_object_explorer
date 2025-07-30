#include "pch.h"
#include "injected_imgui/dx9.h"
#include "injected_imgui/internal.h"

#include "gui.h"

using namespace injected_imgui::internal;

namespace injected_imgui::dx9 {

namespace {

bool initalized = false;

/**
 * @brief Initalizes the dx9 hook if it isn't already.
 *
 * @param device The in use d3d9 device.
 * @return True if the hook was successfully initalized, false otherwise.
 */
bool ensure_initalized(IDirect3DDevice9* device) {
    static bool run_once = false;
    if (run_once) {
        return initalized;
    }
    run_once = true;

    IDirect3DSwapChain9* swap_chain = nullptr;
    if (auto ret = device->GetSwapChain(0, &swap_chain) != D3D_OK) {
        LOG(ERROR, "DX9 hook initalization failed: Failed to get swap chain ({})", ret);
        return false;
    }
    const RaiiLambda raii{[&swap_chain]() {
        if (swap_chain != nullptr) {
            swap_chain->Release();
            swap_chain = nullptr;
        }
    }};

    D3DPRESENT_PARAMETERS params;
    if (auto ret = swap_chain->GetPresentParameters(&params) != D3D_OK) {
        LOG(ERROR, "DX9 hook initalization failed: failed to get preset params ({})", ret);
        return false;
    }

    if (!init_win32_backend(params.hDeviceWindow)) {
        return false;
    }

    if (!ImGui_ImplDX9_Init(device)) {
        LOG(ERROR, "DX9 hook initalization failed: ImGui dx9 init failed");
        return false;
    }

    initalized = true;
    return true;
}

}  // namespace

namespace {

// NOLINTNEXTLINE(modernize-use-using)
typedef HRESULT(INJECTED_IMGUI_STDCALL* device_end_scene_func)(IDirect3DDevice9* self);
device_end_scene_func original_device_end_scene;

const constexpr auto DEVICE_END_SCENE_VF_INDEX = 42;
const constexpr std::string_view DEVICE_END_SCENE_NAME = "IDirect3DDevice9::EndScene";

/**
 * @brief Hook for `IDirect3DDevice9::EndScene`, used to inject imgui.
 */
HRESULT INJECTED_IMGUI_STDCALL device_end_scene_hook(IDirect3DDevice9* self) {
    try {
        static bool nested_call_guard = false;
        if (nested_call_guard) {
            return original_device_end_scene(self);
        }
        nested_call_guard = true;
        const RaiiLambda raii{[]() { nested_call_guard = false; }};

        if (ensure_initalized(self)) {
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            live_object_explorer::gui::render();

            ImGui::EndFrame();
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "Exception occured during DX9 render loop: {}", ex.what());
    } catch (...) {
        LOG(ERROR, "Unknown exception occured during DX9 render loop");
    }

    return original_device_end_scene(self);
}

}  // namespace

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

    HMODULE d3d9_module = GetModuleHandleA("d3d9.dll");
    if (d3d9_module == nullptr) {
        LOG(ERROR, "DX9 hook initalization failed: Couldn't find d3d9.dll");
        return false;
    }

    auto d3d9_create = reinterpret_cast<decltype(Direct3DCreate9)*>(
        GetProcAddress(d3d9_module, "Direct3DCreate9"));
    if (d3d9_create == nullptr) {
        LOG(ERROR, "DX9 hook initalization failed: Couldn't find Direct3DCreate9");
        return false;
    }

    IDirect3D9* d3d = d3d9_create(D3D_SDK_VERSION);
    if (d3d == nullptr) {
        LOG(ERROR, "DX9 hook initalization failed: Couldn't create d3d object");
        return false;
    }
    const RaiiLambda raii3{[&d3d]() {
        if (d3d != nullptr) {
            d3d->Release();
            d3d = nullptr;
        }
    }};

    D3DPRESENT_PARAMETERS params = {
        .BackBufferWidth = 0,
        .BackBufferHeight = 0,
        .BackBufferFormat = D3DFMT_UNKNOWN,
        .BackBufferCount = 0,
        .MultiSampleType = D3DMULTISAMPLE_NONE,
        .MultiSampleQuality = NULL,
        .SwapEffect = D3DSWAPEFFECT_DISCARD,
        .hDeviceWindow = window,
        .Windowed = 1,
        .EnableAutoDepthStencil = 0,
        .AutoDepthStencilFormat = D3DFMT_UNKNOWN,
        .Flags = NULL,
        .FullScreen_RefreshRateInHz = 0,
        .PresentationInterval = 0,
    };

    IDirect3DDevice9* device = nullptr;
    if (auto ret = d3d->CreateDevice(
                       D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, window,
                       D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
                       &params, &device)
                   != D3D_OK) {
        LOG(ERROR, "DX9 hook initalization failed: couldn't create d3d9 device ({})", ret);
        return false;
    }
    const RaiiLambda raii4{[&device]() {
        if (device != nullptr) {
            device->Release();
            device = nullptr;
        }
    }};

    uintptr_t* device_vftable = *reinterpret_cast<uintptr_t**>(device);
    unrealsdk::memory::detour(device_vftable[DEVICE_END_SCENE_VF_INDEX], &device_end_scene_hook,
                              &original_device_end_scene, DEVICE_END_SCENE_NAME);

    return true;
}

}  // namespace injected_imgui::dx9
