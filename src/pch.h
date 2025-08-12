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
#include <unrealsdk/unreal/cast.h>
#include <unrealsdk/unreal/classes/properties/attribute_property.h>
#include <unrealsdk/unreal/classes/properties/copyable_property.h>
#include <unrealsdk/unreal/classes/properties/persistent_object_ptr_property.h>
#include <unrealsdk/unreal/classes/properties/uarrayproperty.h>
#include <unrealsdk/unreal/classes/properties/uboolproperty.h>
#include <unrealsdk/unreal/classes/properties/ubyteproperty.h>
#include <unrealsdk/unreal/classes/properties/uclassproperty.h>
#include <unrealsdk/unreal/classes/properties/ucomponentproperty.h>
#include <unrealsdk/unreal/classes/properties/udelegateproperty.h>
#include <unrealsdk/unreal/classes/properties/uenumproperty.h>
#include <unrealsdk/unreal/classes/properties/uinterfaceproperty.h>
#include <unrealsdk/unreal/classes/properties/umulticastdelegateproperty.h>
#include <unrealsdk/unreal/classes/properties/uobjectproperty.h>
#include <unrealsdk/unreal/classes/properties/ustrproperty.h>
#include <unrealsdk/unreal/classes/properties/ustructproperty.h>
#include <unrealsdk/unreal/classes/properties/utextproperty.h>
#include <unrealsdk/unreal/classes/properties/uweakobjectproperty.h>
#include <unrealsdk/unreal/classes/ublueprintgeneratedclass.h>
#include <unrealsdk/unreal/classes/uclass.h>
#include <unrealsdk/unreal/classes/uconst.h>
#include <unrealsdk/unreal/classes/uenum.h>
#include <unrealsdk/unreal/classes/ufield.h>
#include <unrealsdk/unreal/classes/ufunction.h>
#include <unrealsdk/unreal/classes/uobject.h>
#include <unrealsdk/unreal/classes/uproperty.h>
#include <unrealsdk/unreal/classes/uscriptstruct.h>
#include <unrealsdk/unreal/classes/ustruct.h>
#include <unrealsdk/unreal/find_class.h>
#include <unrealsdk/unreal/structs/fname.h>
#include <unrealsdk/unreal/structs/fstring.h>
#include <unrealsdk/unreal/structs/tarray.h>
#include <unrealsdk/unreal/wrappers/gobjects.h>
#include <unrealsdk/unreal/wrappers/weak_pointer.h>
#include <unrealsdk/unrealsdk.h>
#include <unrealsdk/utils.h>

#endif

#endif /* PCH_H */
