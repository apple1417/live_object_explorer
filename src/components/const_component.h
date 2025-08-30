#ifndef COMPONENTS_CONST_STR_COMPONENT_H
#define COMPONENTS_CONST_STR_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class ConstStrComponent : public AbstractComponent {
   protected:
    std::string str;

   public:
    /**
     * @brief Creates a new component showing a constant string.
     *
     * @param name The component's name. May include hashes.
     * @param str The string to display.
     */
    ConstStrComponent(std::string&& name, std::string&& str);

    ~ConstStrComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override;
};

// Shows the string as disabled
class ConstDisabledStrComponent : public ConstStrComponent {
   public:
    using ConstStrComponent::ConstStrComponent;

    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
};

// Shows the string as an (uneditable) input text box
class ConstTextComponent : public ConstStrComponent {
   public:
    using ConstStrComponent::ConstStrComponent;

    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_CONST_STR_COMPONENT_H */
