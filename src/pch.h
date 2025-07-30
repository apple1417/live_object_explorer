#ifndef PCH_H
#define PCH_H

// The unrealsdk pch defines NOGDI, but we need it for directx
// Include windows.h ourself so we can get a version without it, and just undef the problematic
// ERROR macro
#define WIN32_LEAN_AND_MEAN
#define WIN32_NO_STATUS
#define NOMINMAX
#include <windows.h>
#undef ERROR

#include <unrealsdk/pch.h>

#include <d3d11.h>
#include <d3d12.h>
#include <d3d9.h>
#include <dxgi1_4.h>

#ifdef __cplusplus
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

// NOLINTNEXTLINE(readability-identifier-naming)
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND h_wnd,
                                                             UINT u_msg,
                                                             WPARAM w_param,
                                                             LPARAM l_param);

#include <unrealsdk/commands.h>
#include <unrealsdk/config.h>
#include <unrealsdk/memory.h>
#include <unrealsdk/unrealsdk.h>
#include <unrealsdk/utils.h>

#endif

#endif /* PCH_H */
