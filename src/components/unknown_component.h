#ifndef COMPONENTS_UNKNOWN_COMPONENT_H
#define COMPONENTS_UNKNOWN_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class UnknownComponent : public AbstractComponent {
   protected:
    std::string msg;

   public:
    /**
     * @brief Creates a new component pointing at an unknown field value.
     *
     * @param name The component's name. May include hashes.
     * @param cls_name The unknown class's name.
     */
    UnknownComponent(std::string&& name, std::string_view cls_name);

    ~UnknownComponent() override = default;
    void draw(const ObjectWindowSettings& settings) override;
};

class UnknownPropertyComponent : public AbstractComponent {
   protected:
    std::string msg;

   public:
    /**
     * @brief Creates a new component pointing at an unknown property value.
     *
     * @param name The component's name. May include hashes.
     * @param cls_name The unknown class's name.
     */
    UnknownPropertyComponent(std::string&& name, std::string_view cls_name);

    ~UnknownPropertyComponent() override = default;
    void draw(const ObjectWindowSettings& settings) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_UNKNOWN_COMPONENT_H */
