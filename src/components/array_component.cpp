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
      addr(addr),
      last_data(nullptr),
      inner_prop(inner_prop),
      was_force_closed(false) {}

namespace {

/**
 * @brief Checks if the pointed at array has changed, and updates the components list as required.
 *
 * @param components The components list to update.
 * @param addr The array being pointed at.
 * @param inner_prop The array's inner property type.
 * @param last_data Pointer to the cached last array data pointer.
 */
void update_components_list(std::vector<std::unique_ptr<AbstractComponent>>& components,
                            TArray<void>* addr,
                            UProperty* inner_prop,
                            void** last_data) {
    // If the array data changed, we need to remove all components, since some may keep pointers
    // to the now invalid data
    auto old_count = components.size();
    if (addr->data != *last_data) {
        *last_data = addr->data;
        components.clear();
        old_count = 0;
    }

    // If the count changed, add/remove components as needed
    // In this case, as long as the data pointer is valid, all components should support their
    // contents being swapped out from under them, so we don't need to care about where exactly
    // was modified.
    auto current_count = addr->size();
    if (current_count < old_count) {
        components.erase(components.begin() + (ptrdiff_t)current_count, components.end());
    } else if (current_count > old_count) {
        for (size_t i = old_count; i < current_count; i++) {
            insert_component_array(components, addr, inner_prop, i);
        }
    }
}

/**
 * @brief Deletes elements in a TArray by index.
 *
 * @param indexes_to_remove The list of indexes to remove. Must be ordered.
 * @param addr The array to delete in.
 * @param inner_prop The array's inner property type.
 */
void delete_array_indexes(std::vector<size_t>& indexes_to_remove,
                          TArray<void>* addr,
                          UProperty* inner_prop) {
    if (indexes_to_remove.empty()) {
        return;
    }

    WrappedArray arr{inner_prop, addr};
    auto element_size = inner_prop->ElementSize();

    for (auto [num_removed, idx_to_remove] : std::views::enumerate(indexes_to_remove)) {
        // In the rare case we somehow get multiple removes in a single tick, we know
        // they'll all be ordered, so this deals with indexes shifting on remove.
        idx_to_remove -= num_removed;

        cast(arr.type, [&arr, idx_to_remove]<typename T>(const T* /*prop*/) {
            arr.destroy_at<T>(idx_to_remove);
        });

        // Don't bother moving if deleting the end of the array
        auto size = arr.size();
        if (idx_to_remove != (size - 1)) {
            auto data = static_cast<uint8_t*>(arr.base->data);

            auto dest = &data[idx_to_remove * element_size];
            auto src = &data[(idx_to_remove + 1) * element_size];
            auto remaining_size = (size - (idx_to_remove + 1)) * element_size;

            memmove(dest, src, remaining_size);
        }

        arr.resize(size - 1);
    }
}

}  // namespace

void ArrayComponent::draw(const ObjectWindowSettings& settings,
                          ForceExpandTree expand_children,
                          bool show_all_children) {
    // This is mostly the same as a struct
    show_all_children = show_all_children || AbstractComponent::passes_filter(settings.filter);

    if (expand_children != ForceExpandTree::NONE) {
        ImGui::SetNextItemOpen(expand_children == ForceExpandTree::OPEN);
        this->was_force_closed = expand_children == ForceExpandTree::CLOSE;
    }

    if (ImGui::TreeNodeEx(this->name.c_str(), ImGuiTreeNodeFlags_DrawLinesFull, "(%zu) %s",
                          this->addr->size(), this->name.c_str())) {
        ImGui::TableNextColumn();

        update_components_list(this->components, this->addr, this->inner_prop, &this->last_data);

        std::vector<size_t> indexes_to_remove;
        for (const auto& [idx, component] : std::views::enumerate(this->components)) {
            if (show_all_children || component->passes_filter(settings.filter)) {
                ImGui::PushID(&component);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (settings.editable && ImGui::Button("Remove")) {
                    indexes_to_remove.push_back(idx);
                }

                component->draw(settings,
                                this->was_force_closed ? ForceExpandTree::CLOSE : expand_children,
                                show_all_children);

                ImGui::PopID();
            }
        }

        delete_array_indexes(indexes_to_remove, this->addr, this->inner_prop);

        if (settings.editable) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (ImGui::Button("Add New")) {
                this->addr->resize(this->addr->size() + 1, this->inner_prop->ElementSize());
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
