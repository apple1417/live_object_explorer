#ifndef COMPONENTS_OBJECT_COMPONENT_H
#define COMPONENTS_OBJECT_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class ObjectComponent : public AbstractComponent {
   private:
    std::string name;
    std::string cached_obj_name = "null";
    uintptr_t* addr;
    uintptr_t cached_obj = 0;

   public:
    /**
     * @brief Creates a new component pointing at an object.
     *
     * @param name The name of the property this is wrapping.
     * @param addr Pointer to the object pointer being displayed.
     */
    ObjectComponent(std::string_view name, unrealsdk::unreal::UObject** addr);

    ~ObjectComponent() override = default;
    void draw(const ObjectWindowSettings& settings) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_OBJECT_COMPONENT_H */
