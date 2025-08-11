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

    if (!settings.editable) {
        ImGui::BeginDisabled();
        ImGui::Checkbox(this->name.c_str(), &set);
        ImGui::EndDisabled();
        return;
    }

    ImGui::Checkbox(this->name.c_str(), &set);

    if (set) {
        *this->addr |= this->mask;
    } else {
        *this->addr &= ~this->mask;
    }
}

}  // namespace live_object_explorer
