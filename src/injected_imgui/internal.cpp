#include "pch.h"
#include "injected_imgui/internal.h"
#include "injected_imgui/hook.h"

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

}  // namespace injected_imgui::internal
