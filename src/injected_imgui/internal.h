#ifndef INJECTED_IMGUI_INTERNAL_H
#define INJECTED_IMGUI_INTERNAL_H

namespace injected_imgui::internal {

template <typename F>
struct RaiiLambda {
    F func;

    [[nodiscard]] RaiiLambda(F&& func) : func(std::move(func)) {}
    ~RaiiLambda() { func(); }

    RaiiLambda(const RaiiLambda&) = delete;
    RaiiLambda& operator=(const RaiiLambda&) = delete;
    RaiiLambda(RaiiLambda&&) = delete;
    RaiiLambda& operator=(RaiiLambda&&) = delete;
};

/**
 * @brief Detours a function.
 *
 * @tparam T The signature of the detour'd function (should be picked up automatically).
 * @param addr The address of the function.
 * @param detour_func The detour function.
 * @param original_func Pointer to store the original function.
 * @param name Name of the detour, to be used in log messages on error.
 */
void detour(uintptr_t addr, void* detour_func, void** original_func, std::string_view name);
template <typename T>
void detour(uintptr_t addr, T detour_func, T* original_func, std::string_view name) {
    detour(addr, reinterpret_cast<void*>(detour_func), reinterpret_cast<void**>(original_func),
           name);
}

}  // namespace injected_imgui::internal

#endif /* INJECTED_IMGUI_INTERNAL_H */
