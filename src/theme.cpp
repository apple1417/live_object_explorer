#include "pch.h"
#include "theme.h"

#include <algorithm>
#include "imgui.h"
#include "injected_imgui/auto.h"

namespace live_object_explorer::theme {

namespace {

// There's some bug in the dx12 hook which causes miscolouration on AMD gpus.
// Somehow, we can workaround it by changing all alphas to 254 - which looks ok enough.
// Idk what I'm doing to fix it properly.
// https://github.com/ocornut/imgui/issues/8647
bool apply_amd_dx12_hack = false;

}  // namespace

void init(injected_imgui::auto_detect::Api api) {
    apply_amd_dx12_hack =
        unrealsdk::config::get_bool("live_object_explorer.amd_dx12_hack").value_or(false)
        && api == injected_imgui::auto_detect::Api::DX12;
}

void apply(void) {
    std::string theme{unrealsdk::config::get_str("live_object_explorer.theme").value_or("dark")};
    std::ranges::transform(theme, theme.begin(), [](char chr) { return std::tolower(chr); });

    if (theme == "dark") {
        ImGui::StyleColorsDark();
    } else if (theme == "light") {
        ImGui::StyleColorsLight();
    } else if (theme == "classic") {
        ImGui::StyleColorsClassic();
    } else {
        LOG(WARNING, "Unrecognised theme '{}', using 'dark' instead.", theme);
        ImGui::StyleColorsDark();
    }

    if (apply_amd_dx12_hack) {
        for (auto& colour : ImGui::GetStyle().Colors) {
            if (colour.w == 1.F) {
                // NOLINTNEXTLINE(readability-magic-numbers)
                colour.w = 254.F / 255.F;
            }
        }
    }
}

}  // namespace live_object_explorer::theme
