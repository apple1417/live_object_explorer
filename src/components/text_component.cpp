#include "pch.h"
#include "components/text_component.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

TextComponent::TextComponent(std::string&& name, FText* addr)
    : AbstractComponent(std::move(name)), addr(addr) {}

void TextComponent::draw(const ObjectWindowSettings& /*settings*/,
                         ForceExpandTree /*expand_children*/,
                         bool /*show_all_children*/) {
    // TODO: editable
    ImGui::InputText(this->name.c_str(), this->cached_str.data(), this->cached_str.capacity() + 1,
                     ImGuiInputTextFlags_ReadOnly);
}

bool TextComponent::passes_filter(const ImGuiTextFilter& filter) {
    // Don't have much of a choice other than converting this each tick, we might not catch
    // modifications otherwise
    // Since this function is always called before draw, store it so we only have to convert once
    this->cached_str = *this->addr;

    return AbstractComponent::passes_filter(filter) || filter.PassFilter(this->cached_str.c_str());
}

}  // namespace live_object_explorer
