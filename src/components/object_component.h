#ifndef COMPONENTS_OBJECT_COMPONENT_H
#define COMPONENTS_OBJECT_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class ObjectComponent : public AbstractComponent {
   protected:
    // NOLINTNEXTLINE(readability-redundant-string-init)
    static const constexpr std::string_view NULL_OBJECT_NAME = "";

    std::string cached_obj_name;
    uintptr_t* addr;
    uintptr_t cached_obj;

    unrealsdk::unreal::UClass* property_class;

   public:
    /**
     * @brief Creates a new component pointing at an object.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the object pointer being displayed.
     * @param property_class The class this property must be set to.
     */
    ObjectComponent(std::string&& name,
                    unrealsdk::unreal::UObject** addr,
                    unrealsdk::unreal::UClass* property_class);

    ~ObjectComponent() override = default;
    void draw(const ObjectWindowSettings& settings) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_OBJECT_COMPONENT_H */
