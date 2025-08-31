#include "pch.h"
#include "components/str_component.h"
#include "components/abstract.h"
#include "object_window.h"
#include "string_helper.h"

namespace live_object_explorer {

template <>
StrComponent::GenericStrComponent(std::string&& name, unrealsdk::unreal::UnmanagedFString* addr)
    : AbstractComponent(std::move(name)), addr(addr) {}
template <>
TextComponent::GenericStrComponent(std::string&& name, unrealsdk::unreal::FText* addr)
    : AbstractComponent(std::move(name)), addr(addr) {}

template <typename T>
void GenericStrComponent<T>::draw_impl(const ObjectWindowSettings& settings,
                                       ForceExpandTree /*expand_children*/,
                                       bool /*show_all_children*/) {
    // Don't have much of a choice other than converting this each tick, we might not catch
    // modifications otherwise
    // Slight optimization: if we read it while filtering, we won't need to re-read here
    if (!this->updated_cached_this_tick) {
        this->cached_str = *this->addr;
    }

    ImGui::TextUnformatted(name.c_str());
    ImGui::TableNextColumn();

    int flags = ImGuiInputTextFlags_CallbackResize;
    if (!settings.editable) {
        flags |= ImGuiInputTextFlags_ReadOnly;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputText("##it", this->cached_str.data(), this->cached_str.capacity() + 1, flags,
                         string_resize_callback, &this->cached_str)) {
        // And because we're converting each time, just convert back on any edit too
        *this->addr = this->cached_str;
    }

    this->updated_cached_this_tick = false;
}

template <typename T>
[[nodiscard]] bool GenericStrComponent<T>::passes_filter_impl(const ImGuiTextFilter& filter) {
    this->cached_str = *this->addr;
    this->updated_cached_this_tick = true;

    return AbstractComponent::passes_filter(filter) || filter.PassFilter(this->cached_str.c_str());
}

template <>
void StrComponent::draw(const ObjectWindowSettings& settings,
                        ForceExpandTree expand_children,
                        bool show_all_children) {
    draw_impl(settings, expand_children, show_all_children);
}
template <>
void TextComponent::draw(const ObjectWindowSettings& settings,
                         ForceExpandTree expand_children,
                         bool show_all_children) {
    draw_impl(settings, expand_children, show_all_children);
}

template <>
[[nodiscard]] bool StrComponent::passes_filter(const ImGuiTextFilter& filter) {
    return passes_filter_impl(filter);
}
template <>
[[nodiscard]] bool TextComponent::passes_filter(const ImGuiTextFilter& filter) {
    return passes_filter_impl(filter);
}

}  // namespace live_object_explorer
