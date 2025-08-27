#ifndef COMPONENTS_PERSISTENT_OBJ_PTR_COMPONENT_H
#define COMPONENTS_PERSISTENT_OBJ_PTR_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"
#include "gui/gui.h"

namespace live_object_explorer {

template <typename T>
class PersistentObjectPtrComponent : public AbstractComponent {
   protected:
    std::string hashless_name;
    T* addr;
    unrealsdk::unreal::UClass* property_class;

    std::string identifier;
    std::string cached_obj_name;
    uintptr_t cached_obj = 0;

    /**
     * @brief Tries to set this property to the given object;
     *
     * @param obj The object to check.
     * @return True if setting was allowed/succeeded.
     */
    virtual bool try_set_to_object(unrealsdk::unreal::UObject* obj) const {
        if (obj != nullptr && !obj->is_instance(this->property_class)) {
            return false;
        }

        unrealsdk::gobjects().set_weak_object(&this->addr->weak_ptr, obj);
        return true;
    }

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
              bool /*show_all_children*/) override {
        auto current_obj = unrealsdk::gobjects().get_weak_object(&this->addr->weak_ptr);
        if (this->cached_obj != reinterpret_cast<uintptr_t>(current_obj)) {
            this->cached_obj = reinterpret_cast<uintptr_t>(current_obj);
            if (current_obj != nullptr) {
                this->cached_obj_name =
                    std::format("{}'{}'{}", current_obj->Class()->Name(),
                                unrealsdk::utils::narrow(current_obj->get_path_name()),
                                std::string_view{this->name}.substr(this->length_before_hash));
            }
        }

        // TODO: editable
        ImGui::Text("%s:", this->hashless_name.c_str());
        ImGui::SameLine();
        if (current_obj == nullptr) {
            ImGui::TextDisabled("%s", this->identifier.c_str());
        } else {
            if (ImGui::TextLink(this->cached_obj_name.c_str())) {
                gui::open_object_window(current_obj, this->name);
            }
        }
    }

    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override {
        return AbstractComponent::passes_filter(filter)
               || filter.PassFilter(this->cached_obj_name.c_str())
               || filter.PassFilter(this->identifier.c_str());
    }
};

using SoftObjectComponent = PersistentObjectPtrComponent<unrealsdk::unreal::FSoftObjectPtr>;
using LazyObjectComponent = PersistentObjectPtrComponent<unrealsdk::unreal::FLazyObjectPtr>;

template <>
SoftObjectComponent::PersistentObjectPtrComponent(std::string&& name,
                                                  unrealsdk::unreal::FSoftObjectPtr* addr,
                                                  unrealsdk::unreal::UClass* property_class);
template <>
LazyObjectComponent::PersistentObjectPtrComponent(std::string&& name,
                                                  unrealsdk::unreal::FLazyObjectPtr* addr,
                                                  unrealsdk::unreal::UClass* property_class);

class SoftClassComponent : public SoftObjectComponent {
   protected:
    unrealsdk::unreal::UClass* meta_class;

    bool try_set_to_object(unrealsdk::unreal::UObject* obj) const override;

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
