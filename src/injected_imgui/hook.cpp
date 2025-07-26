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

    return Api::AUTO;
}

}  // namespace

void hook(Api api) {
    if (api == Api::AUTO) {
        api = autodetect_api();
    }

    switch (api) {
        case Api::AUTO:
            throw std::runtime_error("Unable to detect graphics api!");

        case Api::DX12:
            dx12::hook();
            break;

        default:
            throw std::runtime_error("Unimplemented graphics api!");
    }
}

}  // namespace injected_imgui
