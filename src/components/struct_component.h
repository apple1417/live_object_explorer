#ifndef COMPONENTS_STRUCT_COMPONENT_H
#define COMPONENTS_STRUCT_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class StructComponent : public AbstractComponent {
   protected:
    std::vector<std::unique_ptr<AbstractComponent>> components;
    bool was_force_closed;

   public:
    /**
     * @brief Creates a new component pointing at a struct property.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the property.
     * @param ustruct The type of struct being pointed at.
     */
    StructComponent(std::string&& name, uintptr_t addr, unrealsdk::unreal::UStruct* ustruct);

    ~StructComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_STRUCT_COMPONENT_H */
