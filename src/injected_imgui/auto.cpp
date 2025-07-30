#include "pch.h"
#include "injected_imgui/auto.h"
#include "injected_imgui/dx11.h"
#include "injected_imgui/dx12.h"
#include "injected_imgui/dx9.h"

namespace injected_imgui::auto_detect {

std::optional<Api> auto_detect_api(void) {
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

    return std::nullopt;
}

bool hook() {
    auto api = auto_detect_api();
    if (!api) {
        LOG(ERROR, "Failed to autodetect graphics api for imgui hook");
        return false;
    }
    return hook(*api);
}

bool hook(Api api) {
    switch (api) {
        case Api::DX9:
            return dx9::hook();

        case Api::DX11:
            return dx11::hook();

        case Api::DX12:
            return dx12::hook();

        default:
            LOG(ERROR, "Unsupported api for imgui hook");
            return false;
    }
}

}  // namespace injected_imgui::auto_detect
