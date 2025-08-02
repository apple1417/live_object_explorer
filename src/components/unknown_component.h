#ifndef COMPONENTS_UNKNOWN_COMPONENT_H
#define COMPONENTS_UNKNOWN_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class UnknownComponent : public AbstractComponent {
   private:
    std::string name;
    std::string cls_name;

   public:
    /**
     * @brief Creates a new component pointing at an unknown value.
     *
     * @param name The name of the property this is wrapping.
     * @param cls_name The class name of the property this is wrapping.
     */
    UnknownComponent(std::string_view name, std::string_view cls_name);

    ~UnknownComponent() override = default;
    void draw(const ObjectWindowSettings& settings) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_UNKNOWN_COMPONENT_H */
