#include "pch.h"
#include "components/object_component.h"
#include "components/abstract.h"
#include "gui/object.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

ObjectComponent::ObjectComponent(std::string&& name, UObject** addr, UClass* property_class)
    : AbstractComponent(std::move(name)),
      hashless_name(this->name.substr(0, this->length_before_hash)),
      addr(addr),
      property_class(property_class),
      cached_obj(std::string_view{this->name}.substr(this->length_before_hash)) {}

bool ObjectComponent::try_set_to_object(UObject* obj) const {
    if (obj != nullptr && !obj->is_instance(this->property_class)) {
        return false;
    }

    *reinterpret_cast<UObject**>(this->addr) = obj;
    return true;
}

void ObjectComponent::draw(const ObjectWindowSettings& /*settings*/,
                           ForceExpandTree /*expand_children*/,
                           bool /*show_all_children*/) {
    auto current_obj = *this->addr;

    // TODO: editable
    ImGui::Text("%s:", this->hashless_name.c_str());
    ImGui::SameLine();
    this->cached_obj.draw(current_obj, this->name);
}

bool ObjectComponent::passes_filter(const ImGuiTextFilter& filter) {
    return AbstractComponent::passes_filter(filter) || this->cached_obj.passes_filter(filter);
}

namespace {

struct FScriptInterface {
    UObject* obj;     // A pointer to a UObject that implements a native interface.
    void* iface_ptr;  // Pointer to the location of the interface object within the UObject
                      // referenced by ObjectPointer.
};

}  // namespace

bool InterfaceComponent::try_set_to_object(UObject* obj) const {
    size_t pointer_offset = 0;
    if (obj != nullptr) {
        FImplementedInterface impl{};
        if (!obj->is_implementation(this->property_class, &impl)) {
            return false;
        }
        pointer_offset = impl.get_pointer_offset();
    }

    auto iface = reinterpret_cast<FScriptInterface*>(addr);
    iface->obj = obj;
    iface->iface_ptr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(obj) + pointer_offset);
    return true;
}

ClassComponent::ClassComponent(std::string&& name,
                               unrealsdk::unreal::UObject** addr,
                               unrealsdk::unreal::UClass* property_class,
                               unrealsdk::unreal::UClass* meta_class)
    : ObjectComponent(std::move(name), addr, property_class), meta_class(meta_class) {}

bool ClassComponent::try_set_to_object(UObject* obj) const {
    if (obj != nullptr && !obj->is_instance(this->property_class)) {
        return false;
    }
    if (obj != nullptr && !reinterpret_cast<UClass*>(obj)->inherits(this->meta_class)) {
        return false;
    }

    *reinterpret_cast<UObject**>(this->addr) = obj;
    return true;
}

}  // namespace live_object_explorer
