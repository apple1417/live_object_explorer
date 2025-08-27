#include "pch.h"
#include "components/persistent_obj_ptr_component.h"
#include "gui/gui.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

template <>
SoftObjectComponent::PersistentObjectPtrComponent(std::string&& name,
                                                  FSoftObjectPtr* addr,
                                                  UClass* property_class)
    : AbstractComponent(std::move(name)),
      hashless_name(this->name.substr(0, this->length_before_hash)),
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
      hashless_name(this->name.substr(0, this->length_before_hash)),
      addr(addr),
      property_class(property_class),
      identifier(std::format("{:08X}-{:08X}-{:08X}-{:08X}",
                             addr->identifier.guid_a,
                             addr->identifier.guid_b,
                             addr->identifier.guid_c,
                             addr->identifier.guid_d)) {}

SoftClassComponent::SoftClassComponent(std::string&& name,
                                       FSoftObjectPtr* addr,
                                       UClass* property_class,
                                       UClass* meta_class)
    : SoftObjectComponent(std::move(name), addr, property_class), meta_class(meta_class) {}

bool SoftClassComponent::try_set_to_object(unrealsdk::unreal::UObject* obj) const {
    if (obj != nullptr && !obj->is_instance(this->property_class)) {
        return false;
    }
    if (obj != nullptr && !reinterpret_cast<UClass*>(obj)->inherits(this->meta_class)) {
        return false;
    }

    unrealsdk::gobjects().set_weak_object(&this->addr->weak_ptr, obj);
    return true;
}

}  // namespace live_object_explorer
