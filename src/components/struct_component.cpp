#include "pch.h"
#include "components/struct_component.h"
#include "component_picker.h"
#include "components/abstract.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

StructComponent::StructComponent(std::string&& name,
                                 uintptr_t addr,
                                 unrealsdk::unreal::UStruct* ustruct)
    : AbstractComponent(std::move(name)), was_force_closed(false) {
    for (auto field : ustruct->fields()) {
        // We only expect properties, in case we get any fields just stick them in the same list
        insert_component(this->components, this->components, field, addr);
    }
}

void StructComponent::draw(const ObjectWindowSettings& settings,
                           ForceExpandTree expand_children,
                           bool show_all_children) {
    // If the filter matches the struct name, force all children to be displayed
    // If the struct name doesn't match, some children must have, so only show them
    show_all_children = show_all_children || AbstractComponent::passes_filter(settings.filter);

    if (expand_children != ForceExpandTree::NONE) {
        ImGui::SetNextItemOpen(expand_children == ForceExpandTree::OPEN);
        this->was_force_closed = expand_children == ForceExpandTree::CLOSE;
    }

    if (ImGui::TreeNode(this->name.c_str())) {
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

bool StructComponent::passes_filter(const ImGuiTextFilter& filter) {
    // It passes if the root passes, or any child passess
    return AbstractComponent::passes_filter(filter)
           || std::ranges::any_of(this->components, [&filter](auto& component) {
                  return component->passes_filter(filter);
              });
}

}  // namespace live_object_explorer
