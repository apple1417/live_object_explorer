#include "pch.h"
#include "injected_imgui/hook.h"

// TODO: remove when integrating with unrealsdk
#include <chrono>
#include <thread>
#include <iostream>

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
        //std::this_thread::sleep_for(std::chrono::seconds(5));
        volatile bool wait = true;
        while (wait) {}
        injected_imgui::hook();
    } catch (std::exception& ex) {
        std::cerr << "[Live Object Explorer]" << ex.what();
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
