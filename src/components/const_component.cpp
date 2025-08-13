#include "pch.h"
#include "components/const_component.h"
#include "components/abstract.h"

namespace live_object_explorer {

ConstStrComponent::ConstStrComponent(std::string&& name, std::string&& str)
    : AbstractComponent(std::move(name)), str(std::move(str)) {}

void ConstStrComponent::draw(const ObjectWindowSettings& /*settings*/,
                             ForceExpandTree /*expand_children*/,
                             bool /*show_all_children*/) {
    ImGui::Text("%s: %s", this->name.c_str(), this->str.c_str());
}

ConstTextComponent::ConstTextComponent(std::string&& name, std::string&& str)
    : AbstractComponent(std::move(name)), str(std::move(str)) {}

void ConstTextComponent::draw(const ObjectWindowSettings& /*settings*/,
                              ForceExpandTree /*expand_children*/,
                              bool /*show_all_children*/) {
    ImGui::InputText(this->name.c_str(), this->str.data(), this->str.size() + 1,
                     ImGuiInputTextFlags_ReadOnly);
}

}  // namespace live_object_explorer
