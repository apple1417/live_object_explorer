#include "pch.h"
#include "injected_imgui/dx12.h"
#include "injected_imgui/internal.h"

#include "gui.h"

using namespace injected_imgui::internal;

namespace injected_imgui::dx12 {

namespace {

bool initialized = false;

namespace dx {

ID3D12Device* device = nullptr;
ID3D12CommandQueue* command_queue = nullptr;
ID3D12GraphicsCommandList* command_list = nullptr;

struct FrameContext {
    ID3D12CommandAllocator* command_allocator;
    ID3D12Resource* main_render_target_resource;
    D3D12_CPU_DESCRIPTOR_HANDLE main_render_target_descriptor;
};
std::vector<FrameContext> framebuffers;

ID3D12DescriptorHeap* srv_desc_heap = nullptr;
D3D12_CPU_DESCRIPTOR_HANDLE heap_start_cpu;
D3D12_GPU_DESCRIPTOR_HANDLE heap_start_gpu;
size_t heap_increment;
std::vector<size_t> heap_free_indexes;

ID3D12DescriptorHeap* rtv_desc_desc = nullptr;

}  // namespace dx

/**
 * @brief Creates the render resource objects for all frame buffers.
 *
 * @param swap_chain The in use swap chain.
 */
void create_render_resources(IDXGISwapChain* swap_chain) {
    // NOLINTNEXTLINE(modernize-loop-convert)
    for (UINT i = 0; i < dx::framebuffers.size(); i++) {
        swap_chain->GetBuffer(
            i, IID_ID3D12Resource,
            reinterpret_cast<void**>(&dx::framebuffers[i].main_render_target_resource));
        dx::device->CreateRenderTargetView(dx::framebuffers[i].main_render_target_resource, nullptr,
                                           dx::framebuffers[i].main_render_target_descriptor);
    }
}

/**
 * @brief Initializes the dx12 hook if it isn't already.
 *
 * @param swap_chain The in use swap chain.
 * @return True if the hook has been successfully initialized.
 */
bool ensure_initialized(IDXGISwapChain3* swap_chain) {
    static bool run_once = false;
    if (run_once) {
        return initialized;
    }
    run_once = true;

    // The error handling in this function probably leaves something to be desired
    // As long as we've still got a global reference to it it's not a leak right?

    if (auto ret = swap_chain->GetDevice(IID_ID3D12Device, reinterpret_cast<void**>(&dx::device))
                   != S_OK) {
        LOG(ERROR, "DX12 hook initialization failed: Couldn't get device ({})", ret);
        return false;
    }

    UINT buffer_count{};
    HWND h_wnd{};
    {
        DXGI_SWAP_CHAIN_DESC desc;
        if (auto ret = swap_chain->GetDesc(&desc) != S_OK) {
            LOG(ERROR, "DX12 hook initialization failed: Couldn't get swap chain descriptor ({})",
                ret);
            return false;
        }

        buffer_count = desc.BufferCount;
        h_wnd = desc.OutputWindow;

        dx::framebuffers.resize(buffer_count);
    }

    if (!init_win32_backend(h_wnd)) {
        return false;
    }

    {
        const D3D12_DESCRIPTOR_HEAP_DESC srv_desc = {D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                                     buffer_count,
                                                     D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0};

        if (auto ret =
                dx::device->CreateDescriptorHeap(&srv_desc, IID_ID3D12DescriptorHeap,
                                                 reinterpret_cast<void**>(&dx::srv_desc_heap))
                != S_OK) {
            LOG(ERROR, "DX12 hook initialization failed: Couldn't get srv heap descriptor ({})",
                ret);
            return false;
        }

        dx::heap_start_cpu = dx::srv_desc_heap->GetCPUDescriptorHandleForHeapStart();
        dx::heap_start_gpu = dx::srv_desc_heap->GetGPUDescriptorHandleForHeapStart();
        dx::heap_increment =
            dx::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dx::heap_free_indexes.reserve(buffer_count);
        for (size_t i = buffer_count; i > 0; i--) {
            dx::heap_free_indexes.push_back(i - 1);
        }
    }

    {
        const D3D12_DESCRIPTOR_HEAP_DESC rtv_desc = {D3D12_DESCRIPTOR_HEAP_TYPE_RTV, buffer_count,
                                                     D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 1};

        if (auto ret =
                dx::device->CreateDescriptorHeap(&rtv_desc, IID_ID3D12DescriptorHeap,
                                                 reinterpret_cast<void**>(&dx::rtv_desc_desc))
                != S_OK) {
            LOG(ERROR, "DX12 hook initialization failed: Couldn't get srv heap descriptor ({})",
                ret);
            return false;
        }

        const size_t rtv_descriptor_size =
            dx::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle =
            dx::rtv_desc_desc->GetCPUDescriptorHandleForHeapStart();
        for (auto& frame_ctx : dx::framebuffers) {
            frame_ctx.main_render_target_descriptor = rtv_handle;
            rtv_handle.ptr += rtv_descriptor_size;
        }

        create_render_resources(swap_chain);
    }

    {
        // NOLINTNEXTLINE(misc-const-correctness)
        ID3D12CommandAllocator* allocator{};
        if (auto ret = dx::device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                          IID_ID3D12CommandAllocator,
                                                          reinterpret_cast<void**>(&allocator))
                       != S_OK) {
            LOG(ERROR, "DX12 hook initialization failed: Couldn't get command allocator ({})", ret);
            return false;
        }

        for (auto& frame_ctx : dx::framebuffers) {
            frame_ctx.command_allocator = nullptr;
            if (auto ret = dx::device->CreateCommandAllocator(
                               D3D12_COMMAND_LIST_TYPE_DIRECT, IID_ID3D12CommandAllocator,
                               reinterpret_cast<void**>(&frame_ctx.command_allocator))
                           != S_OK) {
                LOG(ERROR,
                    "DX12 hook initialization failed: Couldn't create command allocator ({})", ret);
                return false;
            }
        }
    }

    if (auto ret = dx::device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                 dx::framebuffers[0].command_allocator, nullptr,
                                                 IID_ID3D12GraphicsCommandList,
                                                 reinterpret_cast<void**>(&dx::command_list))
                   != S_OK) {
        LOG(ERROR, "DX12 hook initialization failed: Couldn't create command list ({})", ret);
        return false;
    }

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = dx::device;
    init_info.CommandQueue = dx::command_queue;
    init_info.NumFramesInFlight = (int)buffer_count;
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

    init_info.SrvDescriptorHeap = dx::srv_desc_heap;
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*,
                                        D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
                                        D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
        const size_t idx = dx::heap_free_indexes.back();
        dx::heap_free_indexes.pop_back();
        out_cpu_handle->ptr = dx::heap_start_cpu.ptr + (idx * dx::heap_increment);
        out_gpu_handle->ptr = dx::heap_start_gpu.ptr + (idx * dx::heap_increment);
    };
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*,
                                       D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
                                       D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
        auto cpu_idx = (size_t)((cpu_handle.ptr - dx::heap_start_cpu.ptr) / dx::heap_increment);
        auto gpu_idx = (size_t)((gpu_handle.ptr - dx::heap_start_gpu.ptr) / dx::heap_increment);
        if (cpu_idx != gpu_idx) {
            LOG(ERROR, "DX12 heap free gpu idx was different to cpu");
        }
        dx::heap_free_indexes.push_back(cpu_idx);
    };

    ImGui_ImplDX12_Init(&init_info);

    initialized = true;
    return true;
}

}  // namespace

// =================================================================================================

namespace {

// NOLINTNEXTLINE(modernize-use-using)
typedef void(INJECTED_IMGUI_STDCALL* cmd_queue_exec_func)(ID3D12CommandQueue* self,
                                                          UINT num_command_lists,
                                                          ID3D12CommandList* const* command_lists);
cmd_queue_exec_func original_cmd_queue_exec;

const constexpr auto CMD_QUEUE_EXEC_VF_IDX = 10;
const constexpr std::string_view CMD_QUEUE_EXEC_NAME = "ID3D12CommandQueue::ExecuteCommandLists";

/**
 * @brief Hook for `ID3D12CommandQueue::ExecuteCommandLists`, used to grab the command queue.
 */
void INJECTED_IMGUI_STDCALL cmd_queue_exec_hook(ID3D12CommandQueue* self,
                                                UINT num_command_lists,
                                                ID3D12CommandList* const* command_lists) {
    if (dx::command_queue == nullptr && self->GetDesc().Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
        dx::command_queue = self;
    }

    original_cmd_queue_exec(self, num_command_lists, command_lists);
}

}  // namespace

namespace {

// NOLINTNEXTLINE(modernize-use-using)
typedef HRESULT(INJECTED_IMGUI_STDCALL* swap_chain_present_func)(IDXGISwapChain3* self,
                                                                 UINT sync_interval,
                                                                 UINT flags);
swap_chain_present_func original_swap_chain_present;

const constexpr auto SWAP_CHAIN_PRESENT_VF_INDEX = 8;
const constexpr std::string_view SWAP_CHAIN_PRESENT_NAME = "IDXGISwapChain3::Present";

/**
 * @brief Hook for `IDXGISwapChain3::Present`, used to inject imgui.
 */
HRESULT INJECTED_IMGUI_STDCALL swap_chain_present_hook(IDXGISwapChain3* self,
                                                       UINT sync_interval,
                                                       UINT flags) {
    try {
        static bool nested_call_guard = false;
        if (nested_call_guard) {
            return original_swap_chain_present(self, sync_interval, flags);
        }
        nested_call_guard = true;
        const RaiiLambda raii{[]() { nested_call_guard = false; }};

        if (dx::command_queue != nullptr && ensure_initialized(self)) {
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            live_object_explorer::gui::render();

            ImGui::Render();

            auto& current_frame_context = dx::framebuffers[self->GetCurrentBackBufferIndex()];
            current_frame_context.command_allocator->Reset();

            D3D12_RESOURCE_BARRIER barrier = {
                .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
                .Transition =
                    {
                        .pResource = current_frame_context.main_render_target_resource,
                        .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                        .StateBefore = D3D12_RESOURCE_STATE_PRESENT,
                        .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET,
                    },
            };

            dx::command_list->Reset(current_frame_context.command_allocator, nullptr);
            dx::command_list->ResourceBarrier(1, &barrier);

            dx::command_list->OMSetRenderTargets(
                1, &current_frame_context.main_render_target_descriptor, FALSE, nullptr);
            dx::command_list->SetDescriptorHeaps(1, &dx::srv_desc_heap);

            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dx::command_list);

            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

            dx::command_list->ResourceBarrier(1, &barrier);
            dx::command_list->Close();

            dx::command_queue->ExecuteCommandLists(
                1, reinterpret_cast<ID3D12CommandList**>(&dx::command_list));
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "Exception occurred during DX12 render loop: {}", ex.what());
    } catch (...) {
        LOG(ERROR, "Unknown exception occurred during DX12 render loop");
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
    if (initialized) {
        for (auto& frame : dx::framebuffers) {
            frame.main_render_target_resource->Release();
            frame.main_render_target_resource = nullptr;
        }

        ImGui_ImplDX12_InvalidateDeviceObjects();
    }

    auto ret = original_swap_chain_resize_buffers(self, buffer_count, width, height, new_format,
                                                  swap_chain_flags);

    if (initialized) {
        create_render_resources(self);
    }

    return ret;
}

// =================================================================================================

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

    HMODULE d3d12_module = GetModuleHandleA("d3d12.dll");
    if (d3d12_module == nullptr) {
        LOG(ERROR, "DX12 hook initialization failed: Couldn't find d3d12.dll");
        return false;
    }

    auto d3d12_create_device =
        get_proc_address<decltype(D3D12CreateDevice)>(d3d12_module, "D3D12CreateDevice");
    if (d3d12_create_device == nullptr) {
        LOG(ERROR, "DX12 hook initialization failed: Couldn't find D3D12CreateDevice");
        return false;
    }

    ID3D12Device* hook_device = nullptr;
    if (auto ret = d3d12_create_device(nullptr, D3D_FEATURE_LEVEL_12_0, IID_ID3D12Device,
                                       reinterpret_cast<void**>(&hook_device))
                   != S_OK) {
        LOG(ERROR, "DX12 hook initialization failed: couldn't create d3d12 device ({})", ret);
        return false;
    }
    const RaiiLambda raii3{[&hook_device]() {
        if (hook_device != nullptr) {
            hook_device->Release();
            hook_device = nullptr;
        }
    }};

    const D3D12_COMMAND_QUEUE_DESC queue_desc = {
        .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
        .Priority = 0,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0,
    };

    ID3D12CommandQueue* hook_command_queue = nullptr;
    if (auto ret = hook_device->CreateCommandQueue(&queue_desc, IID_ID3D12CommandQueue,
                                                   reinterpret_cast<void**>(&hook_command_queue))
                   != S_OK) {
        LOG(ERROR, "DX12 hook initialization failed: Couldn't create d3d12 device ({})", ret);
        return false;
    }
    const RaiiLambda raii4{[&hook_command_queue]() {
        if (hook_command_queue != nullptr) {
            hook_command_queue->Release();
            hook_command_queue = nullptr;
        }
    }};

    IDXGIFactory1* factory = nullptr;
    if (auto ret =
            CreateDXGIFactory1(IID_IDXGIFactory1, reinterpret_cast<void**>(&factory)) != S_OK) {
        LOG(ERROR, "DX12 hook initialization failed: Failed to create dxgi factory ({})", ret);
        return false;
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
    if (auto ret =
            factory->CreateSwapChain(hook_command_queue, &swap_chain_desc, &swap_chain) != S_OK) {
        LOG(ERROR, "DX12 hook initialization failed: Failed to create swap chain ({})", ret);
        return false;
    }
    const RaiiLambda raii6{[&swap_chain]() {
        if (swap_chain != nullptr) {
            swap_chain->Release();
            swap_chain = nullptr;
        }
    }};

    const uintptr_t* const command_queue_vftable =
        *reinterpret_cast<uintptr_t**>(hook_command_queue);
    unrealsdk::memory::detour(command_queue_vftable[CMD_QUEUE_EXEC_VF_IDX], &cmd_queue_exec_hook,
                              &original_cmd_queue_exec, CMD_QUEUE_EXEC_NAME);

    const uintptr_t* const swap_chain_vftable = *reinterpret_cast<uintptr_t**>(swap_chain);
    unrealsdk::memory::detour(swap_chain_vftable[SWAP_CHAIN_PRESENT_VF_INDEX],
                              &swap_chain_present_hook, &original_swap_chain_present,
                              SWAP_CHAIN_PRESENT_NAME);
    unrealsdk::memory::detour(swap_chain_vftable[SWAP_CHAIN_RESIZE_BUFFERS_VF_INDEX],
                              &swap_chain_resize_buffers_hook, &original_swap_chain_resize_buffers,
                              SWAP_CHAIN_RESIZE_BUFFERS_NAME);

    return true;
}

}  // namespace injected_imgui::dx12
