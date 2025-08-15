#ifndef COMPONENTS_ENUM_FIELD_COMPONENT_H
#define COMPONENTS_ENUM_FIELD_COMPONENT_H

#include "pch.h"
#include "components/object_field_component.h"

namespace live_object_explorer {

class EnumFieldComponent : public ObjectFieldComponent {
   protected:
    struct EnumNameInfo {
        std::string decimal;
        std::string hex;
    };

    std::vector<EnumNameInfo> name_info;

   public:
    /**
     * @brief Creates a new component pointing at a UEnum as a field.
     *
     * @param name The component's name. May include hashes.
     * @param uenum The enum being displayed.
     */
    EnumFieldComponent(std::string&& name, unrealsdk::unreal::UEnum* uenum);

    ~EnumFieldComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_ENUM_FIELD_COMPONENT_H */
