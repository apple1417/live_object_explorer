#ifndef COMPONENTS_STR_COMPONENT_H
#define COMPONENTS_STR_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

template <typename T>
class GenericStrComponent : public AbstractComponent {
   protected:
    T* addr;
    std::string cached_str;
    bool updated_cached_this_tick = false;

    /**
     * @brief Updates the cached string, if it hasn't already this loop
     */
    void update_cached_str(void) {
        // There's not really an easy way to cache a string, we just need to read it each loop
    }

   public:
    /**
     * @brief Creates a new component pointing at a string property.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the property.
     */
    GenericStrComponent(std::string&& name, T* addr)
        : AbstractComponent(std::move(name)), addr(addr) {}

    ~GenericStrComponent() override = default;

    void draw(const ObjectWindowSettings& /*settings*/,
              ForceExpandTree /*expand_children*/,
              bool /*show_all_children*/) override {
        // Don't have much of a choice other than converting this each tick, we might not catch
        // modifications otherwise
        // Slight optimization: if we read it while filtering, we won't need to re-read here
        if (!this->updated_cached_this_tick) {
            this->cached_str = *this->addr;
        }

        ImGui::TextUnformatted(name.c_str());
        ImGui::TableNextColumn();

        // TODO: editable
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##it", this->cached_str.data(), this->cached_str.capacity() + 1,
                         ImGuiInputTextFlags_ReadOnly);

        this->updated_cached_this_tick = false;
    }

    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override {
        this->cached_str = *this->addr;
        this->updated_cached_this_tick = true;

        return AbstractComponent::passes_filter(filter)
               || filter.PassFilter(this->cached_str.c_str());
    }
};

using StrComponent = GenericStrComponent<unrealsdk::unreal::UnmanagedFString>;
using TextComponent = GenericStrComponent<unrealsdk::unreal::FText>;

}  // namespace live_object_explorer

#endif /* COMPONENTS_STR_COMPONENT_H */
