#include "pch.h"
#include "injected_imgui/internal.h"
#include "injected_imgui/hook.h"

#include "gui.h"

// TODO: remove when integrating unrealsdk
#include <iostream>

namespace injected_imgui::internal {

void detour(uintptr_t addr, void* detour_func, void** original_func, std::string_view name) {
    if (addr == 0) {
        throw inject_error(std::format("detour for {} was passed a null address", name));
    }

    MH_STATUS status = MH_OK;

    status = MH_CreateHook(reinterpret_cast<LPVOID>(addr), detour_func, original_func);
    if (status != MH_OK) {
        const char* error = MH_StatusToString(status);
        throw inject_error(
            std::format("failed to create detour for {} with error: {}", name, error));
    }

    status = MH_EnableHook(reinterpret_cast<LPVOID>(addr));
    if (status != MH_OK) {
        const char* error = MH_StatusToString(status);
        throw inject_error(
            std::format("failed to enable detour for {} with error: {}", name, error));
    }
}

namespace {

WNDPROC window_proc_ptr{};

/**
 * @brief `WinProc` hook, used to pass input to imgui.
 */
LRESULT window_proc_hook(HWND h_wnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {
    bool capture_mouse = false;
    bool capture_kb = false;

    if (live_object_explorer::gui::is_showing()) {
        if (ImGui_ImplWin32_WndProcHandler(h_wnd, u_msg, w_param, l_param) > 0) {
            return 1;
        }

        // NOLINTNEXTLINE(readability-identifier-length)
        auto io = ImGui::GetIO();
        capture_mouse = io.WantCaptureMouse;
        capture_kb = io.WantCaptureKeyboard;
    }

    switch (u_msg) {
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEHWHEEL:
        case WM_MOUSEWHEEL:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_XBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
            if (capture_mouse) {
                return 1;
            }
            break;
        case WM_KEYUP:
        case WM_CHAR:
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            if (capture_kb) {
                return 1;
            }
            break;
    }

    return CallWindowProcA(window_proc_ptr, h_wnd, u_msg, w_param, l_param);
}

}  // namespace

bool init_win32_backend(HWND h_wnd) {
    if (!ImGui_ImplWin32_Init(h_wnd)) {
        std::cerr << "[dhf] hook initalization failed: ImGui win32 init failed!\n";
        return false;
    }

    window_proc_ptr = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrA(h_wnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(window_proc_hook)));

    if (window_proc_ptr == nullptr) {
        std::cerr << "[dhf] hook initalization failed: Failed to replace winproc!\n";
        return false;
    }

    return true;
}

}  // namespace injected_imgui::internal
