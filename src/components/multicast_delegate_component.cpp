#include "pch.h"
#include "components/multicast_delegate_component.h"
#include "components/delegate_component.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

// This is all basically the same as an array

MulticastDelegateComponent::MulticastDelegateComponent(
    std::string&& name,
    unrealsdk::unreal::TArray<unrealsdk::unreal::FScriptDelegate>* addr,
    unrealsdk::unreal::UFunction* signature)
    : AbstractComponent(std::move(name)),
      header(std::format("({}) {}#{}",
                         addr->size(),
                         std::string_view{this->name}.substr(0, this->length_before_hash),
                         std::string_view{this->name}.substr(this->length_before_hash))),
      addr(addr),
      last_data(nullptr),
      signature(signature),
      was_force_closed(false) {}

void MulticastDelegateComponent::draw(const ObjectWindowSettings& settings,
                                      ForceExpandTree expand_children,
                                      bool show_all_children) {
    auto current_count = this->addr->size();
    auto old_count = this->components.size();

    if (current_count != old_count) {
        this->header = std::format("({}) {}#{}", current_count,
                                   std::string_view{this->name}.substr(0, this->length_before_hash),
                                   std::string_view{this->name}.substr(this->length_before_hash));
    }

    show_all_children = show_all_children || AbstractComponent::passes_filter(settings.filter);

    if (expand_children != ForceExpandTree::NONE) {
        ImGui::SetNextItemOpen(expand_children == ForceExpandTree::OPEN);
        this->was_force_closed = expand_children == ForceExpandTree::CLOSE;
    }

    if (ImGui::TreeNode(this->header.c_str())) {
        if (this->addr->data != this->last_data) {
            this->last_data = this->addr->data;
            this->components.clear();
            old_count = 0;
        }

        if (current_count < old_count) {
            this->components.erase(this->components.begin() + (ptrdiff_t)current_count,
                                   this->components.end());
        } else if (current_count > old_count) {
            for (size_t i = old_count; i < current_count; i++) {
                // Main difference: just insert a delegate component directly
                this->components.emplace_back(
                    std::format("[{}]##arr_{}", this->components.size(), this->components.size()),
                    &(*this->addr)[i], this->signature);
            }
        }

        for (auto& component : this->components) {
            if (show_all_children || component.passes_filter(settings.filter)) {
                component.draw(settings,
                               this->was_force_closed ? ForceExpandTree::CLOSE : expand_children,
                               show_all_children);
            }
        }

        this->was_force_closed = false;
        ImGui::TreePop();
    }
}

bool MulticastDelegateComponent::passes_filter(const ImGuiTextFilter& filter) {
    // It passes if the root passes, or any child passess
    return AbstractComponent::passes_filter(filter)
           || std::ranges::any_of(this->components, [&filter](auto& component) {
                  return component.passes_filter(filter);
              });
}

}  // namespace live_object_explorer
