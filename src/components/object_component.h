#ifndef COMPONENTS_OBJECT_COMPONENT_H
#define COMPONENTS_OBJECT_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"
#include "gui/object.h"

namespace live_object_explorer {

class ObjectComponent : public AbstractComponent {
   protected:
    std::string hashless_name;
    unrealsdk::unreal::UObject** addr;
    unrealsdk::unreal::UClass* property_class;

    gui::CachedObjLink cached_obj;

    /**
     * @brief Tries to set this property to the given object;
     *
     * @param obj The object to check.
     * @return True if setting was allowed/succeeded.
     */
    virtual bool try_set_to_object(unrealsdk::unreal::UObject* obj) const;

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
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override;
};

class InterfaceComponent : public ObjectComponent {
   protected:
    bool try_set_to_object(unrealsdk::unreal::UObject* obj) const override;

   public:
    using ObjectComponent::ObjectComponent;

    ~InterfaceComponent() override = default;
};

class ClassComponent : public ObjectComponent {
   protected:
    unrealsdk::unreal::UClass* meta_class;

    bool try_set_to_object(unrealsdk::unreal::UObject* obj) const override;

   public:
    /**
     * @brief Creates a new component pointing at an class.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the object pointer being displayed.
     * @param property_class The class this property must be set to.
     * @param meta_class The class this property must be a subclass of.
     */
    ClassComponent(std::string&& name,
                   unrealsdk::unreal::UObject** addr,
                   unrealsdk::unreal::UClass* property_class,
                   unrealsdk::unreal::UClass* meta_class);

    ~ClassComponent() override = default;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_OBJECT_COMPONENT_H */
