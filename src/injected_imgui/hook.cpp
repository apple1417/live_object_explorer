#include "pch.h"
#include "injected_imgui/hook.h"
#include "injected_imgui/dx12.h"

namespace injected_imgui {

namespace {

/**
 * @brief Attempts to autodetect which graphics api we're using.
 *
 * @return The detected graphics api, or auto if unable to detect.
 */
Api autodetect_api(void) {
    // Go from highest directx down, since later ones can pull in earlier, but not vice versa
    if (GetModuleHandleA("d3d12.dll") != nullptr) {
        return Api::DX12;
    }
    if (GetModuleHandleA("d3d11.dll") != nullptr) {
        return Api::DX11;
    }
    if (GetModuleHandleA("d3d9.dll") != nullptr) {
        return Api::DX9;
    }

    throw inject_error("unable to detect graphics api");
}

}  // namespace

void hook(Api api) {
    if (api == Api::AUTO) {
        api = autodetect_api();
    }

    if (MH_Initialize() != MH_OK) {
        throw inject_error("minhook initialization failed");
    }

    switch (api) {
        case Api::AUTO:
            // should be impossible
            break;

        case Api::DX12:
            dx12::hook();
            break;

        default:
            throw inject_error("unimplemented graphics api");
    }
}

}  // namespace injected_imgui
