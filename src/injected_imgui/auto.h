#ifndef INJECTED_IMGUI_AUTO_H
#define INJECTED_IMGUI_AUTO_H

#include "pch.h"

namespace injected_imgui::auto_detect {

enum class Api : uint8_t {
    DX9,
    DX11,
    DX12,
};

/**
 * @brief Attempts to auto-detect the in use graphics api.
 *
 * @return The detected graphics api, or std::nullopt if unable to detect.
 */
std::optional<Api> auto_detect_api(void);

/**
 * @brief Injects imgui into the current process.
 * @note This may be passed an api explicitly, if you want to detect and hook in two steps.
 *
 * @param api Which graphics api to inject under.
 * @return True if hooked successfully.
 */
bool hook();
bool hook(Api api);

}  // namespace injected_imgui::auto_detect

#endif /* INJECTED_IMGUI_AUTO_H */
