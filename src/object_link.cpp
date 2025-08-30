#include "pch.h"
#include "object_link.h"
#include "gui.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

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

CachedObjLink::CachedObjLink(std::string_view hash)
    : addr(0),
      link_name(NULL_OBJECT_NAME),
      hash(hash),
      length_before_hash(NULL_OBJECT_NAME.size()) {}

void CachedObjLink::update_obj(unrealsdk::unreal::UObject* obj) {
    if (this->addr != reinterpret_cast<uintptr_t>(obj)) {
        this->addr = reinterpret_cast<uintptr_t>(obj);

        if (obj == nullptr) {
            this->link_name = NULL_OBJECT_NAME;
            this->length_before_hash = NULL_OBJECT_NAME.size();

            // Make editable name empty so that the input box shows the hint text instead
            this->editable_name = "";
        } else {
            this->editable_name = format_object_name(obj, "");

            this->link_name = this->editable_name;
            this->length_before_hash = this->link_name.size();
            this->link_name.append(this->hash);
        }
    }
}

void CachedObjLink::draw(unrealsdk::unreal::UObject* obj, const std::string& parent_window_id) {
    update_obj(obj);
    object_link(this->link_name, obj, parent_window_id);
}

namespace {

int string_resize_callback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        auto text = reinterpret_cast<std::string*>(data->UserData);
        IM_ASSERT(data->Buf == text->data());
        text->resize(data->BufTextLen);
        data->Buf = text->data();
    }
    return 0;
}

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
            FName cls{unrealsdk::utils::widen(std::string_view(query.begin(), cls_end))};
            std::string_view object_name{cls_end + 1, query.end() - 1};

            return {cls, unrealsdk::utils::widen(object_name)};
        }
    }

    return {L"Object"_fn, unrealsdk::utils::widen(query)};
}

}  // namespace

void CachedObjLink::draw_editable(unrealsdk::unreal::UObject* obj,
                                  const std::string& label,
                                  const std::function<void(unrealsdk::unreal::UObject*)>& setter) {
    update_obj(obj);

    if (ImGui::InputTextWithHint(
            label.c_str(), NULL_OBJECT_NAME.c_str(), this->editable_name.data(),
            this->editable_name.capacity() + 1,
            ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue,
            string_resize_callback, &this->editable_name)) {
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
    }

    if (!this->failed_to_set_msg.empty()) {
        ImGui::PushID(this);

        bool open = true;
        if (ImGui::BeginPopupModal("Failed to set object", &open,
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", this->failed_to_set_msg.c_str());

            if (ImGui::Button("Close")) {
                open = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (!open) {
            this->failed_to_set_msg.clear();
        }

        ImGui::PopID();
    }
}

void CachedObjLink::fail_to_set(std::string&& msg) {
    this->failed_to_set_msg = std::move(msg);

    // Reload the original name
    if (this->addr == 0) {
        this->editable_name = "";
    } else {
        this->editable_name = this->link_name;
        this->editable_name.resize(this->link_name.size() - this->hash.size());
    }

    ImGui::PushID(this);
    ImGui::OpenPopup("Failed to set object");
    ImGui::PopID();
}

bool CachedObjLink::passes_filter(const ImGuiTextFilter& filter) {
    // If we used editable name, someone could delete the bit that passes the filter while editing
    // Use link name to avoid this - making sure to avoid the hash
    return filter.PassFilter(&this->link_name.c_str()[0],
                             &this->link_name.c_str()[this->length_before_hash]);
}

}  // namespace live_object_explorer
