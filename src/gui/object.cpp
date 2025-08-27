#include "pch.h"
#include "gui/object.h"
#include "gui/gui.h"
#include "imgui.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer::gui {

std::string format_object_name(unrealsdk::unreal::UObject* obj, std::string_view hash) {
    if (obj == nullptr) {
        return std::string{NULL_OBJECT_NAME};
    }
    return std::format("{}'{}'{}", obj->Class()->Name(),
                       unrealsdk::utils::narrow(obj->get_path_name()), hash);
}

namespace {

/**
 * @brief Copies an object name to the clipboard
 *
 * @param text The text to copy.
 */
void copy_to_clipboard(const std::string& text) {
    auto without_hash = text.substr(0, text.rfind("##"));
    ImGui::GetPlatformIO().Platform_SetClipboardTextFn(ImGui::GetCurrentContext(),
                                                       without_hash.c_str());
}

}  // namespace

void object_link(const std::string& text, UObject* obj, const std::string& parent_window_id) {
    if (obj == nullptr) {
        ImGui::TextDisabled("%s", text.c_str());
    } else {
        if (ImGui::TextLink(text.c_str())) {
            if (ImGui::GetIO().KeyShift) {
                copy_to_clipboard(text);
            } else {
                gui::open_object_window(obj, parent_window_id);
            }
        }
    }
}
void object_link(const std::string& text,
                 const std::function<unrealsdk::unreal::UObject*(void)>& obj_getter,
                 const std::string& parent_window_id) {
    if (ImGui::TextLink(text.c_str())) {
        if (ImGui::GetIO().KeyShift) {
            copy_to_clipboard(text);
        } else {
            auto obj = obj_getter();
            if (obj != nullptr) {
                gui::open_object_window(obj, parent_window_id);
            }
        }
    }
}

CachedObjLink::CachedObjLink(std::string_view hash) : addr(0), name(NULL_OBJECT_NAME), hash(hash) {}

void CachedObjLink::draw(unrealsdk::unreal::UObject* obj, const std::string& parent_window_id) {
    if (this->addr != reinterpret_cast<uintptr_t>(obj)) {
        this->addr = reinterpret_cast<uintptr_t>(obj);
        this->name = format_object_name(obj, this->hash);
    }

    object_link(this->name, obj, parent_window_id);
}

bool CachedObjLink::passes_filter(const ImGuiTextFilter& filter) {
    return filter.PassFilter(this->name.c_str());
}

}  // namespace live_object_explorer::gui
