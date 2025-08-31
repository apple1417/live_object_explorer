#include "pch.h"
#include "components/multicast_delegate_component.h"
#include "components/delegate_component.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

// This is all basically the same as an array

MulticastDelegateComponent::MulticastDelegateComponent(std::string&& name,
                                                       TArray<FScriptDelegate>* addr,
                                                       UFunction* signature)
    : AbstractComponent(std::move(name)),
      addr(addr),
      last_data(nullptr),
      signature(signature),
      was_force_closed(false) {}

namespace {

/**
 * @brief Checks if the pointed at array has changed, and updates the components list as required.
 *
 * @param components The components list to update.
 * @param addr The array being pointed at.
 * @param signature The function's signature.
 * @param last_data Pointer to the cached last array data pointer.
 */
void update_components_list(std::vector<DelegateComponent>& components,
                            TArray<FScriptDelegate>* addr,
                            UFunction* signature,
                            void** last_data) {
    auto old_count = components.size();
    if (addr->data != *last_data) {
        *last_data = addr->data;
        components.clear();
        old_count = 0;
    }

    auto current_count = addr->size();
    if (current_count < old_count) {
        components.erase(components.begin() + (ptrdiff_t)current_count, components.end());
    } else if (current_count > old_count) {
        for (size_t i = old_count; i < current_count; i++) {
            // Main difference: just insert a delegate component directly
            components.emplace_back(std::format("[{}]", i), &addr->data[i], signature);
        }
    }
}

/**
 * @brief Deletes elements in a TArray by index.
 *
 * @param indexes_to_remove The list of indexes to remove. Must be ordered.
 * @param addr The array to delete in.
 */
void delete_array_indexes(std::vector<size_t>& indexes_to_remove, TArray<FScriptDelegate>* addr) {
    if (indexes_to_remove.empty()) {
        return;
    }

    for (auto [num_removed, idx_to_remove] : std::views::enumerate(indexes_to_remove)) {
        idx_to_remove -= num_removed;

        // It's a trivial type, no special cleanup needed
        static_assert(std::is_trivially_destructible_v<FScriptDelegate>);

        auto size = addr->size();
        if (idx_to_remove != (size - 1)) {
            auto dest = &addr->data[idx_to_remove];
            auto src = &addr->data[idx_to_remove + 1];
            auto remaining_size = (size - (idx_to_remove + 1)) * sizeof(FScriptDelegate);

            memmove(dest, src, remaining_size);
        }

        // Zero out the last element, so if someone adds a new one we don't get old data
        memset(&addr[size - 1], 0, sizeof(FScriptDelegate));

        addr->resize(size - 1);
    }
}

}  // namespace

void MulticastDelegateComponent::draw(const ObjectWindowSettings& settings,
                                      ForceExpandTree expand_children,
                                      bool show_all_children) {
    show_all_children = show_all_children || AbstractComponent::passes_filter(settings.filter);

    if (expand_children != ForceExpandTree::NONE) {
        ImGui::SetNextItemOpen(expand_children == ForceExpandTree::OPEN);
        this->was_force_closed = expand_children == ForceExpandTree::CLOSE;
    }

    if (ImGui::TreeNodeEx(this->name.c_str(), ImGuiTreeNodeFlags_DrawLinesFull, "(%zu) %s",
                          this->addr->size(), this->name.c_str())) {
        ImGui::TableNextColumn();

        update_components_list(this->components, this->addr, this->signature, &this->last_data);

        std::vector<size_t> indexes_to_remove;
        for (const auto& [idx, component] : std::views::enumerate(this->components)) {
            if (show_all_children || component.passes_filter(settings.filter)) {
                ImGui::PushID(&component);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (settings.editable && ImGui::Button("Remove")) {
                    indexes_to_remove.push_back(idx);
                }

                component.draw(settings,
                               this->was_force_closed ? ForceExpandTree::CLOSE : expand_children,
                               show_all_children);

                ImGui::PopID();
            }
        }

        delete_array_indexes(indexes_to_remove, this->addr);

        if (settings.editable) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (ImGui::Button("Add New")) {
                auto old_size = this->addr->size();
                this->addr->resize(old_size + 1);
                this->addr->data[old_size] = {};
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
