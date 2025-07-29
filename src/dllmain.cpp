#include "pch.h"
#include "gui.h"
#include "injected_imgui/hook.h"
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

        std::optional<injected_imgui::Api> api{};
        if (auto api_str = unrealsdk::config::get_str("live_object_explorer.api")) {
            if (*api_str == "dx9") {
                api = injected_imgui::Api::DX9;
            } else if (*api_str == "dx11") {
                api = injected_imgui::Api::DX11;
            } else if (*api_str == "dx12") {
                api = injected_imgui::Api::DX12;
            } else {
                LOG(ERROR, "Live Object Explorer: Unrecognised graphics api: {}", *api_str);
            }
        }

        if (!api) {
            api = injected_imgui::autodetect_api();
            if (!api) {
                LOG(ERROR, "Live Object Explorer: Failed to autodetect graphics api");
                return 1;
            }
        }

        injected_imgui::hook(*api);

    } catch (const std::exception& ex) {
        LOG(ERROR, "Exception occured during live object explorer initalization: {}", ex.what());
    } catch (...) {
        LOG(ERROR, "Unknown exception occured during live object explorer initalization");
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
