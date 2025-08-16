#ifndef COMPONENTS_NAME_COMPONENT_H
#define COMPONENTS_NAME_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class NameComponent : public AbstractComponent {
   protected:
    unrealsdk::unreal::FName* addr;
    unrealsdk::unreal::FName cached_name;
    std::string cached_str;

   public:
    /**
     * @brief Creates a new component pointing at a name property.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the property.
     */
    NameComponent(std::string&& name, unrealsdk::unreal::FName* addr);

    ~NameComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_NAME_COMPONENT_H */
