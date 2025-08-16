#ifndef COMPONENTS_OBJECT_COMPONENT_H
#define COMPONENTS_OBJECT_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class ObjectComponent : public AbstractComponent {
   public:
    // NOLINTNEXTLINE(readability-redundant-string-init)
    static const constexpr std::string_view NULL_OBJECT_NAME = "None";

   protected:
    std::string hashless_name;
    std::string cached_obj_name;
    uintptr_t* addr;
    uintptr_t cached_obj;

    unrealsdk::unreal::UClass* property_class;

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
