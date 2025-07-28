#include "pch.h"
#include "injected_imgui/hook.h"
#include "injected_imgui/dx11.h"
#include "injected_imgui/dx12.h"
#include "injected_imgui/dx9.h"

namespace injected_imgui {

std::optional<Api> autodetect_api(void) {
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

bool hook(Api api) {
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(
        ::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // NOLINTNEXTLINE(readability-identifier-length)
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

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

}  // namespace injected_imgui
