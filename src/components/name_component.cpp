#include "pch.h"
#include "components/name_component.h"
#include "object_window.h"
#include "string_helper.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

NameComponent::NameComponent(std::string&& name, FName* addr)
    : AbstractComponent(std::move(name)), addr(addr), last_name(-1, -1), pending_edit(false) {}

void NameComponent::draw(const ObjectWindowSettings& settings,
                         ForceExpandTree /*expand_children*/,
                         bool /*show_all_children*/) {
    auto current_name = *this->addr;
    if (current_name != this->last_name) {
        this->last_name = current_name;
        this->cached_str = current_name;
        this->pending_edit = false;
    }

    ImGui::TextUnformatted(this->name.c_str());
    ImGui::TableNextColumn();

    auto flags = ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue;
    if (!settings.editable) {
        flags |= ImGuiInputTextFlags_ReadOnly;
    }

    // While an edit's pending, always use the active bg colour, so it's obvious if you go select
    // something else without submitting.
    if (this->pending_edit) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg,
                              ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_FrameBgActive]);
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputText("##it", this->cached_str.data(), this->cached_str.capacity() + 1, flags,
                         string_resize_callback, &this->cached_str)
        && settings.editable) {
        if (this->pending_edit) {
            ImGui::PopStyleColor();
        }
        this->pending_edit = false;

        *this->addr = FName{this->cached_str};

        // Invalidate last and get a new value next loop, to catch some edge cases like the empty
        // string becoming 'None', so not updating cached_str if it was already 'None' before
        this->last_name = FName{-1, -1};

    } else if (settings.editable) {
        if (this->pending_edit) {
            ImGui::PopStyleColor();
        } else if (ImGui::IsItemEdited()) {
            this->pending_edit = true;
        }
    }
}

bool NameComponent::passes_filter(const ImGuiTextFilter& filter) {
    // Always pass if an edit is pending to try make sure it doesn't disappear if you're editing the
    // part that matches
    return AbstractComponent::passes_filter(filter) || this->pending_edit
           || filter.PassFilter(this->cached_str.c_str());
}

}  // namespace live_object_explorer
