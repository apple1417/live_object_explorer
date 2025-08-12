#include "pch.h"
#include "components/array_component.h"
#include "component_picker.h"
#include "components/abstract.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

ArrayComponent::ArrayComponent(std::string&& name,
                               unrealsdk::unreal::TArray<void>* addr,
                               unrealsdk::unreal::UProperty* inner_prop)
    : AbstractComponent(std::move(name)),
      header(std::format("({}) {}#{}",
                         addr->size(),
                         std::string_view{this->name}.substr(0, this->length_before_hash),
                         std::string_view{this->name}.substr(this->length_before_hash))),
      addr(addr),
      inner_prop(inner_prop),
      was_force_closed(false) {
    for (size_t i = 0; i < this->addr->size(); i++) {
        insert_component(this->components, this->addr, this->inner_prop, i);
    }
}

void ArrayComponent::draw(const ObjectWindowSettings& settings,
                          ForceExpandTree expand_children,
                          bool show_all_children) {
    auto current_count = this->addr->size();
    auto old_count = this->components.size();

    // Update the header to always have the right size
    if (current_count != old_count) {
        this->header = std::format("({}) {}#{}", current_count,
                                   std::string_view{this->name}.substr(0, this->length_before_hash),
                                   std::string_view{this->name}.substr(this->length_before_hash));
    }

    // This is mostly the same as a struct
    show_all_children = show_all_children || AbstractComponent::passes_filter(settings.filter);

    if (expand_children != ForceExpandTree::NONE) {
        ImGui::SetNextItemOpen(expand_children == ForceExpandTree::OPEN);
        this->was_force_closed = expand_children == ForceExpandTree::CLOSE;
    }

    if (ImGui::TreeNode(this->header.c_str())) {
        // If open, make sure we have the right amount of components
        // Components should be made to support their contents being changed out under them
        // This means we just need to keep our component array the same size, we don't need to care
        // about where exactly items were added/removed
        if (current_count < old_count) {
            this->components.erase(this->components.begin() + (ptrdiff_t)current_count,
                                   this->components.end());
        } else {
            for (size_t i = old_count; i < current_count; i++) {
                insert_component(this->components, this->addr, this->inner_prop, i);
            }
        }

        for (auto& component : this->components) {
            if (show_all_children || component->passes_filter(settings.filter)) {
                component->draw(settings,
                                this->was_force_closed ? ForceExpandTree::CLOSE : expand_children,
                                show_all_children);
            }
        }

        this->was_force_closed = false;
        ImGui::TreePop();
    }
}

bool ArrayComponent::passes_filter(const ImGuiTextFilter& filter) {
    // It passes if the root passes, or any child passess
    return AbstractComponent::passes_filter(filter)
           || std::ranges::any_of(this->components, [&filter](auto& component) {
                  return component->passes_filter(filter);
              });
}

}  // namespace live_object_explorer
