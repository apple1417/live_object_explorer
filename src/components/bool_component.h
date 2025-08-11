#ifndef COMPONENTS_BOOL_COMPONENT_H
#define COMPONENTS_BOOL_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class BoolComponent : public AbstractComponent {
   public:
    using field_mask_type = std::remove_reference_t<
        decltype(((unrealsdk::unreal::UBoolProperty*)nullptr)->FieldMask())>;

   protected:
    field_mask_type* addr;
    field_mask_type mask;

   public:
    /**
     * @brief Creates a new component pointing at a bool property.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the property.
     * @param mask Bitfield mask used by the property.
     */
    BoolComponent(std::string&& name, field_mask_type* addr, field_mask_type mask);

    ~BoolComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_BOOL_COMPONENT_H */
