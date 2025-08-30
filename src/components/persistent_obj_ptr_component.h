#ifndef COMPONENTS_PERSISTENT_OBJ_PTR_COMPONENT_H
#define COMPONENTS_PERSISTENT_OBJ_PTR_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"
#include "object_link.h"

namespace live_object_explorer {

template <typename T>
class PersistentObjectPtrComponent : public AbstractComponent {
   protected:
    T* addr;
    unrealsdk::unreal::UClass* property_class;

    std::string identifier;
    CachedObjLink cached_obj;

    /**
     * @brief Tries to set this property to the given object;
     *
     * @param obj The object to try set.
     */
    virtual void try_set_to_object(unrealsdk::unreal::UObject* obj) {
        if (obj != nullptr && !obj->is_instance(this->property_class)) {
            return;
        }

        unrealsdk::gobjects().set_weak_object(&this->addr->weak_ptr, obj);
    }

    void try_set_to_object_impl(unrealsdk::unreal::UObject* obj);
    void draw_impl(const ObjectWindowSettings& /*settings*/,
                   ForceExpandTree /*expand_children*/,
                   bool /*show_all_children*/);

   public:
    /**
     * @brief Creates a new component pointing at a TPersistentObjectPointer.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the object pointer being displayed.
     * @param property_class The class this property must be set to.
     */
    PersistentObjectPtrComponent(std::string&& name,
                                 T* addr,
                                 unrealsdk::unreal::UClass* property_class);

    ~PersistentObjectPtrComponent() override = default;

    void draw(const ObjectWindowSettings& /*settings*/,
              ForceExpandTree /*expand_children*/,
              bool /*show_all_children*/) override;

    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override {
        return AbstractComponent::passes_filter(filter) || this->cached_obj.passes_filter(filter)
               || filter.PassFilter(this->identifier.c_str());
    }
};

using SoftObjectComponent = PersistentObjectPtrComponent<unrealsdk::unreal::FSoftObjectPtr>;
using LazyObjectComponent = PersistentObjectPtrComponent<unrealsdk::unreal::FLazyObjectPtr>;

template <>
void SoftObjectComponent::draw(const ObjectWindowSettings& settings,
                               ForceExpandTree expand_children,
                               bool show_all_children);
template <>
void LazyObjectComponent::draw(const ObjectWindowSettings& settings,
                               ForceExpandTree expand_children,
                               bool show_all_children);

template <>
SoftObjectComponent::PersistentObjectPtrComponent(std::string&& name,
                                                  unrealsdk::unreal::FSoftObjectPtr* addr,
                                                  unrealsdk::unreal::UClass* property_class);
template <>
LazyObjectComponent::PersistentObjectPtrComponent(std::string&& name,
                                                  unrealsdk::unreal::FLazyObjectPtr* addr,
                                                  unrealsdk::unreal::UClass* property_class);

template <>
void SoftObjectComponent::try_set_to_object(unrealsdk::unreal::UObject* obj);
template <>
void LazyObjectComponent::try_set_to_object(unrealsdk::unreal::UObject* obj);

class SoftClassComponent : public SoftObjectComponent {
   protected:
    unrealsdk::unreal::UClass* meta_class;

    void try_set_to_object(unrealsdk::unreal::UObject* obj) override;

   public:
    /**
     * @brief Creates a new component pointing at a soft class pointer.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the object pointer being displayed.
     * @param property_class The class this property must be set to.
     * @param meta_class The class this property must be a subclass of.
     */
    SoftClassComponent(std::string&& name,
                       unrealsdk::unreal::FSoftObjectPtr* addr,
                       unrealsdk::unreal::UClass* property_class,
                       unrealsdk::unreal::UClass* meta_class);

    ~SoftClassComponent() override = default;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_PERSISTENT_OBJ_PTR_COMPONENT_H */
