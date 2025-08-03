#ifndef COMPONENTS_STR_COMPONENT_H
#define COMPONENTS_STR_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class StrComponent : public AbstractComponent {
   protected:
    unrealsdk::unreal::UnmanagedFString* addr;

   public:
    /**
     * @brief Creates a new component pointing at a string property.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the property.
     */
    StrComponent(std::string&& name, unrealsdk::unreal::UnmanagedFString* addr);

    ~StrComponent() override = default;
    void draw(const ObjectWindowSettings& settings) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_STR_COMPONENT_H */
