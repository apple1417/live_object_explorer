#ifndef COMPONENTS_STRUCT_FIELD_COMPONENT_H
#define COMPONENTS_STRUCT_FIELD_COMPONENT_H

#include "pch.h"
#include "components/object_field_component.h"

namespace live_object_explorer {

class StructFieldComponent : public ObjectFieldComponent {
   protected:
    using ObjectFieldComponent::ObjectFieldComponent;

    struct PropData {
        std::string name;
        std::string type;
        unrealsdk::unreal::WeakPointer link;
    };

    std::vector<PropData> properties;
    bool is_function;

   public:
    /**
     * @brief Creates a new component pointing at a UStruct as a field.
     *
     * @param name The component's name. May include hashes.
     * @param ustruct The struct being displayed.
     */
    StructFieldComponent(std::string&& name, unrealsdk::unreal::UStruct* ustruct);

    ~StructFieldComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_STRUCT_FIELD_COMPONENT_H */
