#include "pch.h"
#include "gui.h"
#include "components/abstract.h"
#include "object_window.h"
#include "refs.h"

#include "version.inl"

using namespace unrealsdk::unreal;

namespace live_object_explorer::gui {

namespace {

const constexpr auto GIT_HASH_CHARS = 8;
const std::string TITLE_STR = std::format("Live Object Explorer (v{}, {}{})###Live Object Explorer",
                                          PROJECT_VERSION_MAJOR,
                                          std::string_view(GIT_HEAD_SHA1).substr(0, GIT_HASH_CHARS),
                                          GIT_IS_DIRTY ? ", dirty" : "");

bool search_window_open = false;
bool search_for_refs = false;

using time_point = std::chrono::time_point<std::chrono::steady_clock>;
time_point last_snapshot_time = time_point::min();
time_point next_time_text_update = time_point::min();
std::string last_snapshot_time_text = "Never";

// NOLINTNEXTLINE(readability-magic-numbers)
std::array<char, 1024> search_query{};

std::vector<std::pair<std::string, WeakPointer>> search_results{};
size_t selected_search_idx = 0;
ImGuiTextFilter search_filter;

void do_search(void) {
    search_filter.Clear();
    search_results.clear();
    selected_search_idx = 0;

    std::string_view search{search_query.data()};
    auto first_non_space =
        std::ranges::find_if_not(search, [](auto chr) { return std::isspace(chr); });
    auto [last_non_space, _] = std::ranges::find_last_if_not(
        first_non_space, search.end(), [](auto chr) { return std::isspace(chr); });
    search = std::string_view(first_non_space, last_non_space + 1);

    auto search_wstr = unrealsdk::utils::widen(search);

    UClass* cls = nullptr;
    if (search.find_first_of(".:") == std::wstring::npos) {
        cls = find_class(FName{search_wstr});
    } else {
        UObject* obj = unrealsdk::find_object(L"UObject"_fn, search_wstr);
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

}  // namespace

void search(std::string_view query) {
    auto size = std::min(query.size(), search_query.size() - 1);
    memcpy(search_query.data(), query.data(), size);
    search_query.at(size) = '\0';

    do_search();
}

namespace {

/**
 * @brief Draws the search window, if applicable.
 */
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void draw_search_window(void) {
    if (!search_window_open) {
        return;
    }

    const constexpr auto default_window_size = ImVec2{500, 600};
    ImGui::SetNextWindowSize(default_window_size, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(TITLE_STR.c_str(), &search_window_open)) {
        auto text_size = ImGui::CalcTextSize("Search");
        // The text width, plus one spacing either side
        auto rhs_width = text_size.x + (2 * ImGui::GetStyle().ItemSpacing.x);

        ImGui::SetNextItemWidth(-rhs_width);
        if (ImGui::InputText(
                "##search_bar", search_query.data(), search_query.size(),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
            do_search();
        }
        ImGui::SameLine();
        if (ImGui::Button("Search", ImVec2{-FLT_MIN, 0})) {
            do_search();
        }

        // If we zero the window padding, the header won't extend past the normal margins
        auto old_padding = ImGui::GetCurrentWindow()->WindowPadding.x;
        ImGui::GetCurrentWindow()->WindowPadding.x = 0;

        if (ImGui::TreeNodeEx("References",
                              ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
            ImGui::GetCurrentWindow()->WindowPadding.x = old_padding;

            ImGui::BeginDisabled(!refs::has_snapshot());
            ImGui::Checkbox("Search for References", &search_for_refs);
            if (!refs::has_snapshot()) {
                ImGui::SetItemTooltip("Take a snapshot to be able to search for references");
            }
            ImGui::EndDisabled();

            if (refs::has_snapshot()) {
                auto now = std::chrono::steady_clock::now();
                if (next_time_text_update <= now) {
                    auto minutes =
                        std::chrono::duration_cast<std::chrono::minutes>(now - last_snapshot_time)
                            .count();
                    if (minutes == 0) {
                        last_snapshot_time_text = "Now";
                    } else if (minutes == 1) {
                        last_snapshot_time_text = "1 minute ago";
                    } else {
                        last_snapshot_time_text = std::format("{} minutes ago", minutes);
                    }
                    next_time_text_update = now + std::chrono::minutes{1};
                }
            }

            ImGui::Text("Last Snapshot: %s", last_snapshot_time_text.c_str());
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x
                                 - ImGui::CalcTextSize("Snapshot References").x
                                 - ImGui::GetStyle().ItemSpacing.x);
            if (ImGui::Button("Snapshot References")) {
                LOG(MISC, "Taking snapshot");
                refs::take_snapshot();
                LOG(MISC, "Snapshot finished");
                last_snapshot_time = next_time_text_update = std::chrono::steady_clock::now();
            }

            ImGui::TextWrapped("Taking a snapshot will freeze the game for several seconds.");

            static const bool show_debug =
                unrealsdk::config::get_bool("live_object_explorer.db_debug")
#ifdef NDEBUG
                    .value_or(false);
#else
                    .value_or(true);
#endif
            if (show_debug) {
                ImGui::SeparatorText("Debug");
                if (ImGui::Button("Import DB")) {
                    refs::import_db();
                    last_snapshot_time = next_time_text_update = std::chrono::steady_clock::now();
                }
                ImGui::SameLine();
                ImGui::BeginDisabled(!refs::has_snapshot());
                if (ImGui::Button("Export DB")) {
                    refs::export_db();
                }
                ImGui::EndDisabled();
                const uint32_t thread_min = 1;
                const uint32_t thread_max = 32;
                ImGui::SliderScalar("Threads", ImGuiDataType_U32, &refs::num_threads, &thread_min,
                                    &thread_max);
            }
        } else {
            ImGui::GetCurrentWindow()->WindowPadding.x = old_padding;
        }

        // Assuming text height + a padding each side internally + a padding each side externally
        // Doesn't seem entirely accurate, but at least avoids the scrollbar
        auto below_listbox_height = text_size.y + (4 * ImGui::GetStyle().FramePadding.y);

        if (ImGui::BeginListBox("##search_results", ImVec2{-FLT_MIN, -below_listbox_height})) {
            for (size_t i = 0; i < search_results.size(); i++) {
                auto& [name, ptr] = search_results.at(i);
                if (!search_filter.PassFilter(name.c_str())) {
                    continue;
                }

                const bool is_selected = selected_search_idx == i;
                const bool still_loaded = static_cast<bool>(ptr);

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

ImVec2 get_default_object_window_size(void) {
    const constexpr auto default_x = 600;
    const constexpr auto default_y = 600;

    return {
        static_cast<float>(unrealsdk::config::get_int("live_object_explorer.default_window_size.x")
                               .value_or(default_x)),
        static_cast<float>(unrealsdk::config::get_int("live_object_explorer.default_window_size.y")
                               .value_or(default_y))};
}

}  // namespace

void open_object_window(const FFieldVariant& var) {
    object_windows.emplace_back(var);
    // Intentionally may dock to itself - seem to be required?
    dock_latest_obj_window(object_windows.front().get_id());
}

void open_object_window(const FFieldVariant& var, const std::string& parent_window_id) {
    object_windows.emplace_back(var);
    dock_latest_obj_window(parent_window_id);
}

void show(void) {
    search_window_open = true;
}

bool is_open(void) {
    return search_window_open || !object_windows.empty();
}

void render(void) {
    if (!is_open()) {
        return;
    }

#ifndef NDEBUG
    ImGui::ShowDemoWindow();
#endif

    draw_search_window();

    auto iter = object_windows.begin();
    while (iter != object_windows.end()) {
        bool open = true;

        static const auto default_window_size = get_default_object_window_size();
        ImGui::SetNextWindowSize(default_window_size, ImGuiCond_FirstUseEver);
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
