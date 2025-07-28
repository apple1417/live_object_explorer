#ifndef INJECTED_IMGUI_HOOK_H
#define INJECTED_IMGUI_HOOK_H

#include "pch.h"

namespace injected_imgui {

enum class Api : uint8_t {
    DX9,
    DX11,
    DX12

};

/**
 * @brief Attempts to autodetect the in use graphics api.
 *
 * @return The detected graphics api, or std::nullopt if unable to detect.
 */
std::optional<Api> autodetect_api(void);

/**
 * @brief Injects imgui into the current process.
 *
 * @param api Which graphics api to inject under.
 * @return True if hooked successfully.
 */
bool hook(Api api);

}  // namespace injected_imgui

#endif /* INJECTED_IMGUI_HOOK_H */
