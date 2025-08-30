#include "pch.h"
#include "components/bool_component.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

BoolComponent::BoolComponent(std::string&& name, field_mask_type* addr, field_mask_type mask)
    : AbstractComponent(std::move(name)), addr(addr), mask(mask) {}

void BoolComponent::draw(const ObjectWindowSettings& settings,
                         ForceExpandTree /*expand_children*/,
                         bool /*show_all_children*/) {
    bool set = ((*this->addr) & this->mask) != 0;

    ImGui::TextUnformatted(this->name.c_str());
    ImGui::TableNextColumn();

    ImGui::BeginDisabled(!settings.editable);
    ImGui::Checkbox("##cb", &set);
    ImGui::EndDisabled();

    if (settings.editable) {
        if (set) {
            *this->addr |= this->mask;
        } else {
            *this->addr &= ~this->mask;
        }
    }
}

}  // namespace live_object_explorer
