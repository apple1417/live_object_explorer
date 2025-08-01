#include "pch.h"
#include "gui.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer::gui {

namespace {

bool search_window_open = false;

// NOLINTNEXTLINE(readability-magic-numbers)
char search_query[4096] = "";

std::vector<std::pair<std::string, WeakPointer>> search_results{};
size_t selected_search_idx = 0;
ImGuiTextFilter search_filter;

void do_search(void) {
    search_results.clear();
    selected_search_idx = 0;

    auto first_non_space =
        std::ranges::find_if_not(search_query, [](auto chr) { return std::isspace(chr); });
    auto [last_non_space, _] =
        std::ranges::find_last_if_not(first_non_space, std::ranges::end(search_query),
                                      [](auto chr) { return std::isspace(chr); });
    auto search = unrealsdk::utils::widen(std::string_view(first_non_space, last_non_space));

    UClass* cls = nullptr;
    if (search.find_first_of(L".:") == std::wstring::npos) {
        cls = find_class(FName{search});
    } else {
        UObject* obj = unrealsdk::find_object(L"UObject"_fn, search);
        if (obj == nullptr) {
            return;
        }
        if (!obj->is_instance(find_class<UClass>())) {
            LOG(DEV_WARNING, "TODO DUMP OBJECT {}", search);
            return;
        }
        cls = reinterpret_cast<UClass*>(obj);
    }

    if (cls == nullptr) {
        return;
    }

    std::ranges::copy(unrealsdk::gobjects() | std::views::filter([cls](auto obj) {
                          return obj->is_instance(cls);
                      }) | std::views::transform([](auto obj) {
                          return std::make_pair<std::string, WeakPointer>(
                              unrealsdk::utils::narrow(obj->get_path_name()), obj);
                      }),
                      std::back_inserter(search_results));
}

/**
 * @brief Draws the search window, if applicable.
 */
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void draw_search_window(void) {
    if (!search_window_open) {
        return;
    }

    if (ImGui::Begin("Live Object Explorer", &search_window_open)) {
        auto text_size = ImGui::CalcTextSize("Search");
        auto rhs_width = text_size.x + (2 * ImGui::GetStyle().ItemSpacing.x);
        // Assume the filter box heigh is the same as the general text height
        auto filter_height = text_size.y + (3 * ImGui::GetStyle().FramePadding.y) + 1;

        ImGui::SetNextItemWidth(-rhs_width);
        if (ImGui::InputText(
                "##search_bar", &search_query[0], IM_ARRAYSIZE(search_query),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
            do_search();
        }
        ImGui::SameLine();
        if (ImGui::Button("Search", ImVec2{-FLT_MIN, 0})) {
            do_search();
        }

        if (ImGui::BeginListBox("##search_results", ImVec2{-FLT_MIN, -filter_height})) {
            for (size_t i = 0; i < search_results.size(); i++) {
                auto& [name, ptr] = search_results[i];
                if (!search_filter.PassFilter(name.c_str())) {
                    continue;
                }

                bool is_selected = selected_search_idx == i;
                bool still_loaded = (bool)ptr;

                if (ImGui::Selectable(name.c_str(), is_selected,
                                      still_loaded ? 0 : ImGuiSelectableFlags_Disabled)) {
                    selected_search_idx = i;
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }

                if (still_loaded && ImGui::IsItemHovered()
                    && (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
                        || ImGui::IsKeyPressed(ImGuiKey_Enter))) {
                    LOG(DEV_WARNING, "TODO DUMP OBJECT {}", name);
                }
            }
            ImGui::EndListBox();
        }

        search_filter.Draw("Filter", -rhs_width);
    }
    ImGui::End();
}

}  // namespace

void show(void) {
    search_window_open = true;
}

void render(void) {
    if (!search_window_open) {
        return;
    }

    ImGui::ShowDemoWindow();

    draw_search_window();
}

}  // namespace live_object_explorer::gui
