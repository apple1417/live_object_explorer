#include "pch.h"
#include "injected_imgui/hook.h"

namespace {

HMODULE this_module;

/**
 * @brief Main startup thread.
 * @note Instance of `LPTHREAD_START_ROUTINE`.
 *
 * @return unused.
 */
DWORD WINAPI startup_thread(LPVOID /*unused*/) {
    try {
        // TODO: swap for command
        while (!unrealsdk::is_console_ready()) {}

        if (auto api = injected_imgui::autodetect_api()) {
            injected_imgui::hook(*api);
        } else {
            LOG(ERROR, "Failed to autodetect graphics api");
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "Exception occured during live object explorer initalization: {}", ex.what());
    } catch (...) {
        LOG(ERROR, "Unknown exception occured during live object explorer initalization");
    }

    return 1;
}

}  // namespace

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
            this_module = h_module;
            DisableThreadLibraryCalls(h_module);
            CreateThread(nullptr, 0, &startup_thread, nullptr, 0, nullptr);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
