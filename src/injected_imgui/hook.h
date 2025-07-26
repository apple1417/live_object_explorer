#ifndef INJECTED_IMGUI_HOOK_H
#define INJECTED_IMGUI_HOOK_H

#include "pch.h"

namespace injected_imgui {

enum class Api : uint8_t {
    AUTO,
    DX9,
    DX11,
    DX12,
};

/**
 * @brief Injects imgui into the current process.
 *
 * @param api Which graphics api to inject under.
 */
void hook(Api api = Api::AUTO);

}  // namespace injected_imgui

#endif /* INJECTED_IMGUI_HOOK_H */
