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
#include <list>

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
#include <unrealsdk/unreal/classes/ublueprintgeneratedclass.h>
#include <unrealsdk/unreal/classes/uclass.h>
#include <unrealsdk/unreal/classes/uconst.h>
#include <unrealsdk/unreal/classes/uenum.h>
#include <unrealsdk/unreal/classes/ufield.h>
#include <unrealsdk/unreal/classes/ufunction.h>
#include <unrealsdk/unreal/classes/uobject.h>
#include <unrealsdk/unreal/classes/uscriptstruct.h>
#include <unrealsdk/unreal/classes/ustruct.h>
#include <unrealsdk/unreal/find_class.h>
#include <unrealsdk/unreal/properties/attribute_property.h>
#include <unrealsdk/unreal/properties/copyable_property.h>
#include <unrealsdk/unreal/properties/persistent_object_ptr_property.h>
#include <unrealsdk/unreal/properties/zarrayproperty.h>
#include <unrealsdk/unreal/properties/zboolproperty.h>
#include <unrealsdk/unreal/properties/zbyteproperty.h>
#include <unrealsdk/unreal/properties/zclassproperty.h>
#include <unrealsdk/unreal/properties/zcomponentproperty.h>
#include <unrealsdk/unreal/properties/zdelegateproperty.h>
#include <unrealsdk/unreal/properties/zenumproperty.h>
#include <unrealsdk/unreal/properties/zinterfaceproperty.h>
#include <unrealsdk/unreal/properties/zmulticastdelegateproperty.h>
#include <unrealsdk/unreal/properties/zobjectproperty.h>
#include <unrealsdk/unreal/properties/zproperty.h>
#include <unrealsdk/unreal/properties/zstrproperty.h>
#include <unrealsdk/unreal/properties/zstructproperty.h>
#include <unrealsdk/unreal/properties/ztextproperty.h>
#include <unrealsdk/unreal/properties/zweakobjectproperty.h>
#include <unrealsdk/unreal/structs/ffield.h>
#include <unrealsdk/unreal/structs/fname.h>
#include <unrealsdk/unreal/structs/fscriptdelegate.h>
#include <unrealsdk/unreal/structs/fstring.h>
#include <unrealsdk/unreal/structs/ftext.h>
#include <unrealsdk/unreal/structs/fweakobjectptr.h>
#include <unrealsdk/unreal/structs/tarray.h>
#include <unrealsdk/unreal/structs/tfieldvariant.h>
#include <unrealsdk/unreal/structs/tpersistentobjectptr.h>
#include <unrealsdk/unreal/wrappers/bound_function.h>
#include <unrealsdk/unreal/wrappers/gobjects.h>
#include <unrealsdk/unreal/wrappers/weak_pointer.h>
#include <unrealsdk/unreal/wrappers/wrapped_array.h>
#include <unrealsdk/unrealsdk.h>
#include <unrealsdk/utils.h>

#endif

#endif /* PCH_H */
