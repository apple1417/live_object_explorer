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
};

class ConstTextComponent : public AbstractComponent {
   protected:
    std::string str;

   public:
    /**
     * @brief Creates a new component showing a string in an uneditable text box.
     *
     * @param name The component's name. May include hashes.
     * @param str The string to display.
     */
    ConstTextComponent(std::string&& name, std::string&& str);

    ~ConstTextComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_CONST_STR_COMPONENT_H */
