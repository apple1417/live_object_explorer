#include "pch.h"
#include "components/delegate_component.h"
#include "components/abstract.h"
#include "gui.h"
#include "imgui.h"

namespace live_object_explorer {

DelegateComponent::DelegateComponent(std::string&& name,
                                     unrealsdk::unreal::FScriptDelegate* addr,
                                     unrealsdk::unreal::UFunction* signature)
    : AbstractComponent(std::move(name)),
      addr(addr),
      signature(signature),
      hashless_name(this->name.substr(0, this->length_before_hash)) {}

void DelegateComponent::draw(const ObjectWindowSettings& /*settings*/,
                             ForceExpandTree /*expand_children*/,
                             bool /*show_all_children*/) {
    auto obj = this->addr->get_object();
    if (obj != this->last_obj) {
        this->last_obj = obj;
        this->cached_obj_name =
            obj == nullptr ? ""
                           : this->cached_obj_name = std::format(
                                 "{}'{}'{}", obj->Class()->Name(),
                                 unrealsdk::utils::narrow(obj->get_path_name()),
                                 std::string_view{this->name}.substr(this->length_before_hash));
    }
    if (this->addr->func_name != this->last_func_name) {
        this->last_func_name = this->addr->func_name;
        this->cached_func_name =
            std::format("{}{}", this->last_func_name,
                        std::string_view{this->name}.substr(this->length_before_hash));
    }

    ImGui::Text("%s:", this->hashless_name.c_str());
    ImGui::SameLine();
    if (obj == nullptr) {
        ImGui::TextDisabled("None");
    } else {
        if (ImGui::TextLink(this->cached_func_name.c_str())) {
            gui::open_object_window(obj->Class()->find(this->last_func_name), this->name);
        }
        ImGui::SameLine();
        ImGui::Text("on");
        ImGui::SameLine();
        if (ImGui::TextLink(this->cached_obj_name.c_str())) {
            gui::open_object_window(obj, this->name);
        }
    }
}

}  // namespace live_object_explorer
