#include "pch.h"
#include "components/delegate_component.h"
#include "components/abstract.h"
#include "object_link.h"

namespace live_object_explorer {

DelegateComponent::DelegateComponent(std::string&& name,
                                     unrealsdk::unreal::FScriptDelegate* addr,
                                     unrealsdk::unreal::UFunction* signature)
    : AbstractComponent(std::move(name)), addr(addr), signature(signature) {}

void DelegateComponent::draw(const ObjectWindowSettings& /*settings*/,
                             ForceExpandTree /*expand_children*/,
                             bool /*show_all_children*/) {
    auto current_obj = this->addr->get_object();

    if (this->addr->func_name != this->last_func_name) {
        this->last_func_name = this->addr->func_name;
        this->cached_func_name = this->last_func_name;
    }

    ImGui::TextUnformatted(this->name.c_str());
    ImGui::TableNextColumn();

    if (current_obj == nullptr) {
        ImGui::TextDisabled("None");
    } else {
        // TODO: editable
        object_link(this->last_func_name,
                    [&]() { return current_obj->Class()->find(this->last_func_name); });
        ImGui::SameLine();
        ImGui::Text("on");
        ImGui::SameLine();
        this->cached_obj.draw(current_obj);
    }
}

bool DelegateComponent::passes_filter(const ImGuiTextFilter& filter) {
    return AbstractComponent::passes_filter(filter)
           || filter.PassFilter(this->cached_func_name.c_str())
           || this->cached_obj.passes_filter(filter);
}

}  // namespace live_object_explorer
