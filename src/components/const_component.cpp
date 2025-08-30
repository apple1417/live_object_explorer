#include "pch.h"
#include "components/const_component.h"
#include "components/abstract.h"

namespace live_object_explorer {

ConstStrComponent::ConstStrComponent(std::string&& name, std::string&& str)
    : AbstractComponent(std::move(name)), str(std::move(str)) {}

void ConstStrComponent::draw(const ObjectWindowSettings& /*settings*/,
                             ForceExpandTree /*expand_children*/,
                             bool /*show_all_children*/) {
    ImGui::TextUnformatted(this->name.c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(this->str.c_str());
}

bool ConstStrComponent::passes_filter(const ImGuiTextFilter& filter) {
    return AbstractComponent::passes_filter(filter) || filter.PassFilter(this->str.c_str());
}

void ConstDisabledStrComponent::draw(const ObjectWindowSettings& /*settings*/,
                                     ForceExpandTree /*expand_children*/,
                                     bool /*show_all_children*/) {
    ImGui::TextUnformatted(this->name.c_str());
    ImGui::TableNextColumn();
    ImGui::TextDisabled("%s", this->str.c_str());
}

void ConstTextComponent::draw(const ObjectWindowSettings& /*settings*/,
                              ForceExpandTree /*expand_children*/,
                              bool /*show_all_children*/) {
    ImGui::TextUnformatted(this->name.c_str());
    ImGui::TableNextColumn();

    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##it", this->str.data(), this->str.capacity() + 1,
                     ImGuiInputTextFlags_ReadOnly);
}

}  // namespace live_object_explorer
