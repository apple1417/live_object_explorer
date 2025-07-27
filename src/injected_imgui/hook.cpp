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

    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(
        ::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // NOLINTNEXTLINE(readability-identifier-length)
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(
        main_scale);  // Bake a fixed style scale. (until we have a solution for dynamic style
                      // scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi =
        main_scale;  // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this
                     // unnecessary. We leave both here for documentation purpose)
    io.ConfigDpiScaleFonts =
        true;  // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor
               // DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    io.ConfigDpiScaleViewports =
        true;  // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look
    // identical to regular ones.
    if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0) {
        style.WindowRounding = 0.0;
        style.Colors[ImGuiCol_WindowBg].w = 1.0;
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
