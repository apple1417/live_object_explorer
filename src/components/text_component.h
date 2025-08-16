#ifndef COMPONENTS_TEXT_COMPONENT_H
#define COMPONENTS_TEXT_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class TextComponent : public AbstractComponent {
   protected:
    unrealsdk::unreal::FText* addr;
    std::string cached_str;

   public:
    /**
     * @brief Creates a new component pointing at a text property.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the property.
     */
    TextComponent(std::string&& name, unrealsdk::unreal::FText* addr);

    ~TextComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_TEXT_COMPONENT_H */
