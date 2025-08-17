#include "pch.h"
#include "gui.h"
#include "components/abstract.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer::gui {

namespace {

const constexpr auto DEFAULT_WINDOW_SIZE = ImVec2{400, 500};

bool search_window_open = false;

// NOLINTNEXTLINE(readability-magic-numbers)
char search_query[4096] = "";

std::vector<std::pair<std::string, WeakPointer>> search_results{};
size_t selected_search_idx = 0;
ImGuiTextFilter search_filter;

void do_search(void) {
    search_filter.Clear();
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
            open_object_window(obj);
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

    ImGui::SetNextWindowSize(DEFAULT_WINDOW_SIZE, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Live Object Explorer", &search_window_open)) {
        auto text_size = ImGui::CalcTextSize("Search");
        auto rhs_width = text_size.x + (2 * ImGui::GetStyle().ItemSpacing.x);
        // Assume the filter box height is the same as the general text height
        auto filter_height = text_size.y + (3 * ImGui::GetStyle().FramePadding.y) + 1;

        ImGui::SetNextItemWidth(-rhs_width);
        if (ImGui::InputText(
                "##search_bar", &search_query[0], (sizeof(search_query) / sizeof(search_query[0])),
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

                const bool is_selected = selected_search_idx == i;
                const bool still_loaded = (bool)ptr;

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
                    open_object_window(*ptr);
                }
            }
            ImGui::EndListBox();
        }

        search_filter.Draw("Filter", -rhs_width);
    }
    ImGui::End();
}

}  // namespace

namespace {

// Use a list since we mostly care about deleting items in the middle without invalidating iterators
std::list<ObjectWindow> object_windows{};

/**
 * @brief Docks the latest opened object window to the given window.
 *
 * @param parent_window_id The id of the window to dock to.
 */
void dock_latest_obj_window(const std::string& /* parent_window_id */) {
    // TODO
}

}  // namespace

void open_object_window(UObject* obj) {
    object_windows.emplace_back(obj);
    // Intentionally may dock to itself - seem to be required?
    dock_latest_obj_window(object_windows.front().get_id());
}

void open_object_window(UObject* obj, const std::string& parent_window_id) {
    object_windows.emplace_back(obj);
    dock_latest_obj_window(parent_window_id);
}

void show(void) {
    search_window_open = true;
}

void render(void) {
    if (!search_window_open && object_windows.empty()) {
        return;
    }

#ifndef NDEBUG
    ImGui::ShowDemoWindow();
#endif

    draw_search_window();

    auto iter = object_windows.begin();
    while (iter != object_windows.end()) {
        bool open = true;

        ImGui::SetNextWindowSize(DEFAULT_WINDOW_SIZE, ImGuiCond_FirstUseEver);
        if (ImGui::Begin(iter->get_id().c_str(), &open,
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)) {
            iter->draw();
        }
        ImGui::End();

        if (!open) {
            ImGui::ClearWindowSettings(iter->get_id().c_str());
            iter = object_windows.erase(iter);
        } else {
            iter++;
        }
    }
}

}  // namespace live_object_explorer::gui
