#include "pch.h"
#include "components/delegate_component.h"
#include "components/abstract.h"
#include "object_link.h"
#include "object_window.h"
#include "string_helper.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

DelegateComponent::DelegateComponent(std::string&& name,
                                     FScriptDelegate* addr,
                                     UFunction* signature)
    : AbstractComponent(std::move(name)),
      addr(addr),
      signature(signature),
      func_name_pending_edit(false) {}

void DelegateComponent::draw_editable(UObject* current_obj) {
    /*
    A delegate consists of two parts: the function name and the object.
    We use two boxes for this, rather than trying to come up with some new syntax (which'd also
    require re-implementing a lot of CachedObjLink::draw_editable).
    This then gives us the question of how do you safely change both, without leaving the delegate
    in an invalid state inbetween.
    It's easy enough to swap out just one of the two, the problem's doing both - most likely when
    going from none to something.

    The choice I make is to let you arbitrarily set the function name as long as there's no object.
    Editing the object always needs to leave you with a valid bound function.
    Since a delegate only stores the function name, it has to look up the actual function at
    execution time, which requires the object, so thinking is hopefully the engine uses "no object"
    checks first, meaning "no object" + "has function name" is still treated the same.
    */

    if (this->func_name_pending_edit) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg,
                              ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_FrameBgActive]);
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputTextWithHint(
            "##fn", NULL_OBJECT_NAME.c_str(), this->cached_func_name.data(),
            this->cached_func_name.capacity() + 1,
            ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue,
            string_resize_callback, &this->cached_func_name)) {
        if (this->func_name_pending_edit) {
            ImGui::PopStyleColor();
        }
        this->func_name_pending_edit = false;

        if (current_obj == nullptr) {
            // If the object is null, allow setting the name to anything
            this->addr->func_name = FName{this->cached_func_name};
        } else {
            // Otherwise, validate and bind as normal, it must exist on this object
            try {
                auto bound_func =
                    current_obj->get<UFunction, BoundFunction>(FName{this->cached_func_name});
                FScriptDelegate::validate_signature(bound_func, this->signature);
                this->addr->bind(bound_func);
            } catch (const std::exception& ex) {
                if (this->cached_func_name.empty()) {
                    this->cached_obj.fail_to_set(std::format(
                        "{}\nIf you were trying to clear the delegate, delete the object instead.",
                        ex.what()));
                } else {
                    this->cached_obj.fail_to_set(ex.what());
                }

                // Restore the original name
                this->cached_func_name = this->last_func_name;
            }
        }
    } else {
        if (this->func_name_pending_edit) {
            ImGui::PopStyleColor();
        } else if (ImGui::IsItemEdited()) {
            this->func_name_pending_edit = true;
        }
    }

    ImGui::TextUnformatted("on");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    this->cached_obj.draw_editable(current_obj, [this](UObject* obj) {
        if (obj == nullptr) {
            this->addr->bind(std::nullopt);
            return;
        }

        try {
            auto bound_func = obj->get<UFunction, BoundFunction>(this->last_func_name);
            FScriptDelegate::validate_signature(bound_func, this->signature);
            this->addr->bind(bound_func);
        } catch (const std::exception& ex) {
            this->cached_obj.fail_to_set(ex.what());
        }
    });
}

void DelegateComponent::draw(const ObjectWindowSettings& settings,
                             ForceExpandTree /*expand_children*/,
                             bool /*show_all_children*/) {
    auto current_obj = this->addr->get_object();

    if (this->addr->func_name != this->last_func_name) {
        this->last_func_name = this->addr->func_name;

        if (this->last_func_name == FName{0, 0}) {
            this->cached_func_name = "";
        } else {
            this->cached_func_name = this->last_func_name;
        }
        this->func_name_pending_edit = false;
    }

    ImGui::TextUnformatted(this->name.c_str());
    ImGui::TableNextColumn();

    if (settings.editable) {
        this->draw_editable(current_obj);
    } else if (current_obj == nullptr) {
        ImGui::TextDisabled("%s", NULL_OBJECT_NAME.c_str());
    } else {
        object_link(this->cached_func_name,
                    [&]() { return current_obj->Class()->find(this->last_func_name); });
        ImGui::SameLine();
        ImGui::Text("on");
        ImGui::SameLine();
        this->cached_obj.draw(current_obj);
    }
}

bool DelegateComponent::passes_filter(const ImGuiTextFilter& filter) {
    return AbstractComponent::passes_filter(filter)
           || this->cached_obj.passes_filter(filter)
           // Always pass if an edit is pending to try make sure it doesn't disappear if you're
           // editing the part that matches
           || this->func_name_pending_edit || filter.PassFilter(this->cached_func_name.c_str());
}

}  // namespace live_object_explorer
