#ifndef COMPONENTS_STATIC_OBJECT_COMPONENT_H
#define COMPONENTS_STATIC_OBJECT_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class ObjectFieldComponent : public AbstractComponent {
   protected:
    unrealsdk::unreal::WeakPointer ptr;
    std::string hashless_name;
    std::string cached_obj_name;

   public:
    /**
     * @brief Creates a new component pointing at a static object reference.
     *
     * @param name The component's name. May include hashes.
     * @param obj The object being displayed.
     */
    ObjectFieldComponent(std::string&& name, unrealsdk::unreal::UObject* obj);

    ~ObjectFieldComponent() override = default;
    void draw(const ObjectWindowSettings& settings) override;
    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_STATIC_OBJECT_COMPONENT_H */
