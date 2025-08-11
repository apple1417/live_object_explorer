#include "pch.h"
#include "components/str_component.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

StrComponent::StrComponent(std::string&& name, UnmanagedFString* addr)
    : AbstractComponent(std::move(name)), addr(addr) {}

void StrComponent::draw(const ObjectWindowSettings& /*settings*/) {
    // TODO: editable
    ImGui::InputText(this->name.c_str(), this->cached_str.data(), this->cached_str.capacity() + 1,
                     ImGuiInputTextFlags_ReadOnly);
}

bool StrComponent::passes_filter(const ImGuiTextFilter& filter) {
    // Don't have much of a choice other than converting this each tick, we might not catch
    // modifications otherwise
    // Since this function is always called before draw, store it so we only have to convert once
    this->cached_str = *this->addr;

    return AbstractComponent::passes_filter(filter) || filter.PassFilter(this->cached_str.c_str());
}

}  // namespace live_object_explorer
