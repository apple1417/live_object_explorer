#include "pch.h"
#include "components/persistent_obj_ptr_component.h"
#include "gui.h"
#include "object_link.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

template <typename T>
void PersistentObjectPtrComponent<T>::try_set_to_object_impl(unrealsdk::unreal::UObject* obj) {
    if (obj != nullptr && !obj->is_instance(this->property_class)) {
        this->cached_obj.fail_to_set(std::format("Object is not an instance of {}:\n{}",
                                                 this->property_class->Name(),
                                                 obj->get_path_name()));
        return;
    }

    unrealsdk::gobjects().set_weak_object(&this->addr->weak_ptr, obj);
}

template <>
void SoftObjectComponent::try_set_to_object(unrealsdk::unreal::UObject* obj) {
    this->try_set_to_object_impl(obj);
}

template <>
void LazyObjectComponent::try_set_to_object(unrealsdk::unreal::UObject* obj) {
    this->try_set_to_object_impl(obj);
}

template <>
SoftObjectComponent::PersistentObjectPtrComponent(std::string&& name,
                                                  FSoftObjectPtr* addr,
                                                  UClass* property_class)
    : AbstractComponent(std::move(name)),
      addr(addr),
      property_class(property_class),
      identifier(addr->identifier.asset_path_name) {
    auto subpath_size = addr->identifier.subpath.size();
    if (subpath_size > 0) {
        identifier.reserve(identifier.size() + subpath_size + 1);
        name += ':';
        name += (std::string)addr->identifier.subpath;
    }
}

template <>
LazyObjectComponent::PersistentObjectPtrComponent(std::string&& name,
                                                  FLazyObjectPtr* addr,
                                                  UClass* property_class)
    : AbstractComponent(std::move(name)),
      addr(addr),
      property_class(property_class),
      identifier(std::format("{:08X}-{:08X}-{:08X}-{:08X}",
                             addr->identifier.guid_a,
                             addr->identifier.guid_b,
                             addr->identifier.guid_c,
                             addr->identifier.guid_d)) {}

template <typename T>
void PersistentObjectPtrComponent<T>::draw_impl(const ObjectWindowSettings& settings,
                                                ForceExpandTree /*expand_children*/,
                                                bool /*show_all_children*/) {
    auto current_obj = unrealsdk::gobjects().get_weak_object(&this->addr->weak_ptr);

    ImGui::TextUnformatted(this->name.c_str());
    ImGui::TableNextColumn();

    if (settings.editable) {
        ImGui::SetNextItemWidth(-FLT_MIN);
        this->cached_obj.draw_editable(current_obj,
                                       [this](UObject* obj) { this->try_set_to_object(obj); });
    } else {
        this->cached_obj.draw(current_obj);
    }
}

template <>
void SoftObjectComponent::draw(const ObjectWindowSettings& settings,
                               ForceExpandTree expand_children,
                               bool show_all_children) {
    draw_impl(settings, expand_children, show_all_children);
}
template <>
void LazyObjectComponent::draw(const ObjectWindowSettings& settings,
                               ForceExpandTree expand_children,
                               bool show_all_children) {
    draw_impl(settings, expand_children, show_all_children);
}

SoftClassComponent::SoftClassComponent(std::string&& name,
                                       FSoftObjectPtr* addr,
                                       UClass* property_class,
                                       UClass* meta_class)
    : SoftObjectComponent(std::move(name), addr, property_class), meta_class(meta_class) {}

void SoftClassComponent::try_set_to_object(unrealsdk::unreal::UObject* obj) {
    if (obj != nullptr && !obj->is_instance(this->property_class)) {
        this->cached_obj.fail_to_set(std::format("Object is not an instance of {}:\n{}",
                                                 this->property_class->Name(),
                                                 obj->get_path_name()));
    }
    if (obj != nullptr && !reinterpret_cast<UClass*>(obj)->inherits(this->meta_class)) {
        this->cached_obj.fail_to_set(std::format("Object is not a subclass of {}:\n{}",
                                                 this->meta_class->Name(), obj->get_path_name()));
    }

    unrealsdk::gobjects().set_weak_object(&this->addr->weak_ptr, obj);
}

}  // namespace live_object_explorer
