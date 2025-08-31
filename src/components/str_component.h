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

    void draw_impl(const ObjectWindowSettings& settings,
                   ForceExpandTree expand_children,
                   bool show_all_children);
    [[nodiscard]] bool passes_filter_impl(const ImGuiTextFilter& filter);

   public:
    /**
     * @brief Creates a new component pointing at a string property.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the property.
     */
    GenericStrComponent(std::string&& name, T* addr);

    ~GenericStrComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override;
};

using StrComponent = GenericStrComponent<unrealsdk::unreal::UnmanagedFString>;
using TextComponent = GenericStrComponent<unrealsdk::unreal::FText>;

template <>
StrComponent::GenericStrComponent(std::string&& name, unrealsdk::unreal::UnmanagedFString* addr);
template <>
TextComponent::GenericStrComponent(std::string&& name, unrealsdk::unreal::FText* addr);

template <>
void StrComponent::draw(const ObjectWindowSettings& settings,
                        ForceExpandTree expand_children,
                        bool show_all_children);
template <>
void TextComponent::draw(const ObjectWindowSettings& settings,
                         ForceExpandTree expand_children,
                         bool show_all_children);

template <>
[[nodiscard]] bool StrComponent::passes_filter(const ImGuiTextFilter& filter);
template <>
[[nodiscard]] bool TextComponent::passes_filter(const ImGuiTextFilter& filter);

}  // namespace live_object_explorer

#endif /* COMPONENTS_STR_COMPONENT_H */
