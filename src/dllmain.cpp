#include "pch.h"
#include "gui.h"
#include "injected_imgui/auto.h"
#include "theme.h"
#include "unrealsdk/utils.h"

namespace live_object_explorer {

namespace {

/**
 * @brief Main startup thread.
 * @note Instance of `LPTHREAD_START_ROUTINE`.
 *
 * @return unused.
 */
DWORD WINAPI startup_thread(LPVOID /*unused*/) {
    try {
        while (!unrealsdk::is_console_ready()) {}

        auto cmd = unrealsdk::config::get_str("live_object_explorer.command").value_or("explore");
        if (!unrealsdk::commands::add_command(unrealsdk::utils::widen(cmd),
                                              [](const wchar_t* /* line */, size_t /* len */,
                                                 size_t /* cmd_len */) { gui::show(); })) {
            LOG(ERROR, "Live Object Explorer: failed to add '{}' command", cmd);
        }

        std::optional<injected_imgui::auto_detect::Api> api{};
        if (auto api_str = unrealsdk::config::get_str("live_object_explorer.api")) {
            if (*api_str == "dx9") {
                api = injected_imgui::auto_detect::Api::DX9;
            } else if (*api_str == "dx11") {
                api = injected_imgui::auto_detect::Api::DX11;
            } else if (*api_str == "dx12") {
                api = injected_imgui::auto_detect::Api::DX12;
            } else {
                LOG(ERROR, "Live Object Explorer: Unrecognised graphics api: {}", *api_str);
            }
        }

        if (!api) {
            api = injected_imgui::auto_detect::auto_detect_api();
            if (!api) {
                LOG(ERROR, "Live Object Explorer: Failed to autodetect graphics api");
                return 1;
            }
        }

        theme::init(*api);

        ImGui_ImplWin32_EnableDpiAwareness();
        float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(
            ::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // NOLINTNEXTLINE(readability-identifier-length)
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        theme::apply();

        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        injected_imgui::auto_detect::hook(*api);

#ifndef NDEBUG
        gui::show();
#endif

    } catch (const std::exception& ex) {
        LOG(ERROR, "Exception occurred during live object explorer initialization: {}", ex.what());
    } catch (...) {
        LOG(ERROR, "Unknown exception occurred during live object explorer initialization");
    }

    return 1;
}

}  // namespace

}  // namespace live_object_explorer

/**
 * @brief Main entry point.
 *
 * @param h_module Handle to module for this dll.
 * @param ul_reason_for_call Reason this is being called.
 * @return True if loaded successfully, false otherwise.
 */
// NOLINTNEXTLINE(misc-use-internal-linkage, readability-identifier-naming)
BOOL APIENTRY DllMain(HMODULE h_module, DWORD ul_reason_for_call, LPVOID /*unused*/) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(h_module);
            CreateThread(nullptr, 0, &live_object_explorer::startup_thread, nullptr, 0, nullptr);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
