#include "pch.h"
#include "injected_imgui/dx12.h"
#include "injected_imgui/hook.h"
#include "injected_imgui/internal.h"

namespace injected_imgui::dx12 {

using namespace injected_imgui::internal;

namespace {

using cmd_queue_exec_func = void (*)(ID3D12CommandQueue* self,
                                     UINT num_command_lists,
                                     ID3D12CommandList* const* commmand_lists);
cmd_queue_exec_func original_cmd_queue_exec;

const constexpr auto CMD_QUEUE_EXEC_VF_IDX = 10;
const constexpr std::string_view CMD_QUEUE_EXEC_NAME = "ID3D12CommandQueue::ExecuteCommandLists";

/**
 * @brief Hook for `ID3D12CommandQueue::ExecuteCommandLists`, used to grab the command queue.
 */
void cmd_queue_exec_hook(ID3D12CommandQueue* self,
                         UINT num_command_lists,
                         ID3D12CommandList* const* commmand_lists) {
    original_cmd_queue_exec(self, num_command_lists, commmand_lists);
}

}  // namespace

namespace {

using swap_chain_present_func = HRESULT (*)(IDXGISwapChain3* self, UINT sync_interval, UINT flags);
swap_chain_present_func original_swap_chain_present;

const constexpr auto SWAP_CHAIN_PRESENT_VF_INDEX = 8;
const constexpr std::string_view SWAP_CHAIN_PRESENT_NAME = "IDXGISwapChain3::Present";

/**
 * @brief Hook for `IDXGISwapChain3::Present`, used to inject imgui.
 */
HRESULT swap_chain_present_hook(IDXGISwapChain3* self, UINT sync_interval, UINT flags) {
    return original_swap_chain_present(self, sync_interval, flags);
}

}  // namespace

namespace {

using swap_chain_resize_buffers_func = HRESULT (*)(IDXGISwapChain* self,
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
HRESULT swap_chain_resize_buffers_hook(IDXGISwapChain* self,
                                       UINT buffer_count,
                                       UINT width,
                                       UINT height,
                                       DXGI_FORMAT new_format,
                                       UINT swap_chain_flags) {
    return original_swap_chain_resize_buffers(self, buffer_count, width, height, new_format,
                                              swap_chain_flags);
}

}  // namespace

void hook(void) {
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

    HMODULE d3d12_module = GetModuleHandleA("d3d12.dll");
    if (d3d12_module == nullptr) {
        throw inject_error("couldn't find d3d12.dll");
    }

    auto d3d12_create_device = reinterpret_cast<decltype(D3D12CreateDevice)*>(
        GetProcAddress(d3d12_module, "D3D12CreateDevice"));
    if (d3d12_create_device == nullptr) {
        throw inject_error("couldn't find D3D12CreateDevice");
    }

    ID3D12Device* device = nullptr;
    if (d3d12_create_device(nullptr, D3D_FEATURE_LEVEL_12_0, IID_ID3D12Device,
                            reinterpret_cast<void**>(&device))
        != S_OK) {
        throw inject_error("couldn't create d3d12 device");
    }
    const RaiiLambda raii3{[&device]() {
        if (device != nullptr) {
            device->Release();
            device = nullptr;
        }
    }};

    const D3D12_COMMAND_QUEUE_DESC queue_desc = {
        .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
        .Priority = 0,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0,
    };

    ID3D12CommandQueue* command_queue = nullptr;
    if (device->CreateCommandQueue(&queue_desc, IID_ID3D12CommandQueue,
                                   reinterpret_cast<void**>(&command_queue))
        != S_OK) {
        throw inject_error("couldn't create d3d12 device");
    }
    const RaiiLambda raii4{[&command_queue]() {
        if (command_queue != nullptr) {
            command_queue->Release();
            command_queue = nullptr;
        }
    }};

    IDXGIFactory1* factory = nullptr;
    if (CreateDXGIFactory1(IID_IDXGIFactory1, reinterpret_cast<void**>(&factory)) != S_OK) {
        throw inject_error("failed to create dxgi factory");
    }
    const RaiiLambda raii5{[&factory]() {
        if (factory != nullptr) {
            factory->Release();
            factory = nullptr;
        }
    }};

    // NOLINTBEGIN(readability-magic-numbers)
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {
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
        .SampleDesc =
            {
                .Count = 1,
                .Quality = 0,
            },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2,
        .OutputWindow = window,
        .Windowed = 1,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
    };
    // NOLINTEND(readability-magic-numbers)

    IDXGISwapChain* swap_chain = nullptr;
    if (factory->CreateSwapChain(command_queue, &swap_chain_desc, &swap_chain) < S_OK) {
        throw inject_error("failed to create swap chain");
    }
    const RaiiLambda raii6{[&swap_chain]() {
        if (swap_chain != nullptr) {
            swap_chain->Release();
            swap_chain = nullptr;
        }
    }};

    uintptr_t* command_queue_vftable = *reinterpret_cast<uintptr_t**>(command_queue);
    detour(command_queue_vftable[CMD_QUEUE_EXEC_VF_IDX], &cmd_queue_exec_hook,
           &original_cmd_queue_exec, CMD_QUEUE_EXEC_NAME);

    uintptr_t* swap_chain_vftable = *reinterpret_cast<uintptr_t**>(swap_chain);
    detour(swap_chain_vftable[SWAP_CHAIN_PRESENT_VF_INDEX], &swap_chain_present_hook,
           &original_swap_chain_present, SWAP_CHAIN_PRESENT_NAME);
    detour(swap_chain_vftable[SWAP_CHAIN_RESIZE_BUFFERS_VF_INDEX], &swap_chain_resize_buffers_hook,
           &original_swap_chain_resize_buffers, SWAP_CHAIN_RESIZE_BUFFERS_NAME);
}

}  // namespace injected_imgui::dx12
