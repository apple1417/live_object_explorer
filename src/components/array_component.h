#ifndef COMPONENTS_ARRAY_COMPONENT_H
#define COMPONENTS_ARRAY_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class ArrayComponent : public AbstractComponent {
   protected:
    unrealsdk::unreal::TArray<void>* addr;
    void* last_data;
    unrealsdk::unreal::UProperty* inner_prop;
    std::vector<std::unique_ptr<AbstractComponent>> components;
    bool was_force_closed;

   public:
    /**
     * @brief Creates a new component pointing at an array property.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the array.
     * @param inner_prop The array's inner property type.
     */
    ArrayComponent(std::string&& name,
                   unrealsdk::unreal::TArray<void>* addr,
                   unrealsdk::unreal::UProperty* inner_prop);

    ~ArrayComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_ARRAY_COMPONENT_H */
