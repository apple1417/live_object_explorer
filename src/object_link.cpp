#include "pch.h"
#include "object_link.h"
#include "gui.h"
#include "string_helper.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

std::string format_object_name(unrealsdk::unreal::UObject* obj) {
    if (obj == nullptr) {
        return std::string{NULL_OBJECT_NAME};
    }
    return std::format("{}'{}'", obj->Class()->Name(),
                       unrealsdk::utils::narrow(obj->get_path_name()));
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

void object_link(const std::string& text, UObject* obj) {
    if (obj == nullptr) {
        ImGui::TextDisabled("%s", text.c_str());
    } else {
        if (ImGui::TextLink(text.c_str())) {
            gui::open_object_window(obj);
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered()) {
            copy_to_clipboard(text);
        }
    }
}
void object_link(const std::string& text,
                 const std::function<unrealsdk::unreal::UObject*(void)>& obj_getter) {
    if (ImGui::TextLink(text.c_str())) {
        auto obj = obj_getter();
        if (obj != nullptr) {
            gui::open_object_window(obj);
        }
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered()) {
        copy_to_clipboard(text);
    }
}

void CachedObjLink::update_obj(unrealsdk::unreal::UObject* obj) {
    if (this->addr != reinterpret_cast<uintptr_t>(obj)) {
        this->addr = reinterpret_cast<uintptr_t>(obj);
        this->name = format_object_name(obj);

        // Set editable name to the empty string when null so that we get the hint text instead
        this->editable_name = obj == nullptr ? "" : this->name;
        this->pending_edit = false;
    }
}

void CachedObjLink::draw(unrealsdk::unreal::UObject* obj) {
    update_obj(obj);

    ImGui::PushID(this);
    object_link(this->name, obj);
    ImGui::PopID();
}

namespace {

/**
 * @brief Splits an object name into class/name.
 *
 * @param text The text to split.
 * @return A pair of the class and object names.
 */
std::pair<FName, std::wstring> split_class_obj_name(const std::string& text) {
    auto first_non_space =
        std::ranges::find_if_not(text, [](auto chr) { return std::isspace(chr); });
    auto [last_non_space, _] = std::ranges::find_last_if_not(
        first_non_space, std::ranges::end(text), [](auto chr) { return std::isspace(chr); });
    auto query = std::string_view(first_non_space, last_non_space + 1);

    // Look for the class'obj.name' format
    if (query.back() == '\'') {
        auto cls_end = std::ranges::find(query, '\'');
        if (cls_end != query.end() && cls_end != (query.end() - 1)) {
            const FName cls{unrealsdk::utils::widen(std::string_view(query.begin(), cls_end))};
            const std::string_view object_name{cls_end + 1, query.end() - 1};

            return {cls, unrealsdk::utils::widen(object_name)};
        }
    }

    return {L"Object"_fn, unrealsdk::utils::widen(query)};
}

}  // namespace

void CachedObjLink::draw_editable(unrealsdk::unreal::UObject* obj,
                                  const std::function<void(unrealsdk::unreal::UObject*)>& setter) {
    update_obj(obj);

    ImGui::PushID(this);

    // While an edit's pending, always use the active bg colour, so it's obvious if you go select
    // something else without submitting.
    if (this->pending_edit) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg,
                              ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_FrameBgActive]);
    }

    if (ImGui::InputTextWithHint(
            "##it", NULL_OBJECT_NAME.c_str(), this->editable_name.data(),
            this->editable_name.capacity() + 1,
            ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue,
            string_resize_callback, &this->editable_name)) {
        if (this->pending_edit) {
            ImGui::PopStyleColor();
        }
        this->pending_edit = false;

        ImGui::PopID();

        if (this->editable_name.empty()) {
            setter(nullptr);
        } else {
            auto [cls, obj_name] = split_class_obj_name(this->editable_name);

            auto found_obj = unrealsdk::find_object(cls, obj_name);
            if (found_obj == nullptr) {
                this->fail_to_set(
                    std::format("Could not find object\nClass: {}\nName: {}", cls, obj_name));
            } else {
                setter(found_obj);
            }
        }

        ImGui::PushID(this);
    } else {
        if (this->pending_edit) {
            ImGui::PopStyleColor();
        } else if (ImGui::IsItemEdited()) {
            this->pending_edit = true;
        }
    }

    if (!this->failed_to_set_msg.empty()) {
        bool open = true;
        if (ImGui::BeginPopupModal("Failed to set object", &open,
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted(this->failed_to_set_msg.c_str());

            if (ImGui::Button("Close") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                open = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (!open) {
            this->failed_to_set_msg.clear();
        }
    }

    ImGui::PopID();
}

void CachedObjLink::fail_to_set(std::string&& msg) {
    this->failed_to_set_msg = std::move(msg);

    // Reload the original name
    if (this->addr == 0) {
        this->editable_name = "";
    } else {
        this->editable_name = this->name;
    }
    this->pending_edit = false;

    ImGui::PushID(this);
    ImGui::OpenPopup("Failed to set object");
    ImGui::PopID();
}

bool CachedObjLink::passes_filter(const ImGuiTextFilter& filter) {
    return filter.PassFilter(this->name.c_str());
}

}  // namespace live_object_explorer
