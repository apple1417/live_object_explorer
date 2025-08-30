#include "pch.h"
#include "components/name_component.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

NameComponent::NameComponent(std::string&& name, FName* addr)
    : AbstractComponent(std::move(name)), addr(addr) {}

void NameComponent::draw(const ObjectWindowSettings& /*settings*/,
                         ForceExpandTree /*expand_children*/,
                         bool /*show_all_children*/) {
    auto current_name = *this->addr;
    if (current_name != this->cached_name) {
        this->cached_name = current_name;
        this->cached_str = current_name;
    }

    ImGui::TextUnformatted(this->name.c_str());
    ImGui::TableNextColumn();

    // TODO: editable
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##it", this->cached_str.data(), this->cached_str.capacity() + 1,
                     ImGuiInputTextFlags_ReadOnly);
}

bool NameComponent::passes_filter(const ImGuiTextFilter& filter) {
    return AbstractComponent::passes_filter(filter) || filter.PassFilter(this->cached_str.c_str());
}

}  // namespace live_object_explorer
