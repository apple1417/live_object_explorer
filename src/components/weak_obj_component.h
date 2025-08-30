#ifndef COMPONENTS_WEAK_OBJ_COMPONENT_H
#define COMPONENTS_WEAK_OBJ_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"
#include "gui.h"
#include "object_link.h"

namespace live_object_explorer {

class WeakObjectComponent : public AbstractComponent {
   protected:
    unrealsdk::unreal::FWeakObjectPtr* addr;
    unrealsdk::unreal::UClass* property_class;
    CachedObjLink cached_obj;

   public:
    /**
     * @brief Creates a new component pointing at a weak object pointer.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the object pointer being displayed.
     * @param property_class The class this property must be set to.
     */
    WeakObjectComponent(std::string&& name,
                        unrealsdk::unreal::FWeakObjectPtr* addr,
                        unrealsdk::unreal::UClass* property_class);

    ~WeakObjectComponent() override = default;

    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_WEAK_OBJ_COMPONENT_H */
