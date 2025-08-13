#include "pch.h"
#include "components/const_component.h"
#include "components/abstract.h"

namespace live_object_explorer {

ConstStrComponent::ConstStrComponent(std::string&& name, std::string&& str)
    : AbstractComponent(std::move(name)),
      hashless_name(this->name.substr(0, this->length_before_hash)),
      str(std::move(str)) {}

void ConstStrComponent::draw(const ObjectWindowSettings& /*settings*/,
                             ForceExpandTree /*expand_children*/,
                             bool /*show_all_children*/) {
    ImGui::Text("%s: %s", this->hashless_name.c_str(), this->str.c_str());
}

bool ConstStrComponent::passes_filter(const ImGuiTextFilter& filter) {
    return AbstractComponent::passes_filter(filter) || filter.PassFilter(this->str.c_str());
}

void ConstDisabledStrComponent::draw(const ObjectWindowSettings& /*settings*/,
                                     ForceExpandTree /*expand_children*/,
                                     bool /*show_all_children*/) {
    ImGui::Text("%s:", this->hashless_name.c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("%s", this->str.c_str());
}

void ConstTextComponent::draw(const ObjectWindowSettings& /*settings*/,
                              ForceExpandTree /*expand_children*/,
                              bool /*show_all_children*/) {
    ImGui::InputText(this->name.c_str(), this->str.data(), this->str.size() + 1,
                     ImGuiInputTextFlags_ReadOnly);
}

}  // namespace live_object_explorer
