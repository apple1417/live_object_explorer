#ifndef INJECTED_IMGUI_INTERNAL_H
#define INJECTED_IMGUI_INTERNAL_H

#include "pch.h"

namespace injected_imgui::internal {

// Stdcall doesn't exist for 64bit, so with pedantic warnings some compilers throw one
// This define is used to remove it when unnecessary
// Note that MSVC also does not allow a using statement to include calling conventions, so anything
// referencing this is forced to use a typedef
#if UINTPTR_MAX == UINT64_MAX
#define INJECTED_IMGUI_STDCALL
#else
#define INJECTED_IMGUI_STDCALL __stdcall
#endif

template <typename F>
struct RaiiLambda {
    F func;

    [[nodiscard]] RaiiLambda(F&& func) : func(std::move(func)) {}
    ~RaiiLambda() { func(); }

    RaiiLambda(const RaiiLambda&) = delete;
    RaiiLambda& operator=(const RaiiLambda&) = delete;
    RaiiLambda(RaiiLambda&&) = delete;
    RaiiLambda& operator=(RaiiLambda&&) = delete;
};

/**
 * @brief Initalizes the win32 imgui backend for the given window.
 *
 * @param h_wnd Handle to the window to hook.
 * @return True if hooked successfully.
 */
bool init_win32_backend(HWND h_wnd);

}  // namespace injected_imgui::internal

#endif /* INJECTED_IMGUI_INTERNAL_H */
