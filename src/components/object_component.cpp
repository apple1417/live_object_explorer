#include "pch.h"
#include "components/object_component.h"
#include "components/abstract.h"
#include "object_link.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

ObjectComponent::ObjectComponent(std::string&& name, UObject** addr, UClass* property_class)
    : AbstractComponent(std::move(name)),
      addr(reinterpret_cast<void**>(addr)),
      property_class(property_class) {}
ObjectComponent::ObjectComponent(std::string&& name, FField** addr, FFieldClass* property_class)
    : AbstractComponent(std::move(name)),
      addr(reinterpret_cast<void**>(addr)),
      property_class(property_class) {}

void ObjectComponent::try_set_to_object(UObject* obj) {
    if (this->property_class.is_ffield()) {
        this->cached_obj.fail_to_set(std::format("Cannot assign to an FField pointer!"));
        return;
    }
    auto prop_class = this->property_class.as_uobject();

    if (obj != nullptr && !obj->is_instance(prop_class)) {
        this->cached_obj.fail_to_set(std::format("Object is not an instance of {}:\n{}",
                                                 prop_class->Name(), obj->get_path_name()));
        return;
    }

    *reinterpret_cast<UObject**>(this->addr) = obj;
}

void ObjectComponent::draw(const ObjectWindowSettings& settings,
                           ForceExpandTree /*expand_children*/,
                           bool /*show_all_children*/) {
    FFieldVariant var{};
    if (this->property_class.is_ffield()) {
        var = *reinterpret_cast<FField**>(this->addr);
    } else {
        var = *reinterpret_cast<UObject**>(this->addr);
    }

    ImGui::TextUnformatted(this->name.c_str());
    ImGui::TableNextColumn();

    if (settings.editable) {
        ImGui::SetNextItemWidth(-FLT_MIN);
        this->cached_obj.draw_editable(var, [this](UObject* obj) { this->try_set_to_object(obj); });
    } else {
        this->cached_obj.draw(var);
    }
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

InterfaceComponent::InterfaceComponent(std::string&& name,
                                       unrealsdk::unreal::UObject** addr,
                                       unrealsdk::unreal::UClass* property_class)
    : ObjectComponent(std::move(name), addr, property_class) {}

void InterfaceComponent::try_set_to_object(UObject* obj) {
    if (this->property_class.is_ffield()) {
        // Shouldn't be possible, the constructor only takes uobjects
        this->cached_obj.fail_to_set(std::format("Cannot assign to an FField pointer!"));
        return;
    }
    auto prop_class = this->property_class.as_uobject();

    size_t pointer_offset = 0;
    if (obj != nullptr) {
        FImplementedInterface impl{};
        if (!obj->is_implementation(prop_class, &impl)) {
            this->cached_obj.fail_to_set(std::format("Object is not an implementation of {}:\n{}",
                                                     prop_class->Name(), obj->get_path_name()));
            return;
        }
        pointer_offset = impl.get_pointer_offset();
    }

    auto iface = reinterpret_cast<FScriptInterface*>(addr);
    iface->obj = obj;
    iface->iface_ptr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(obj) + pointer_offset);
}

ClassComponent::ClassComponent(std::string&& name,
                               unrealsdk::unreal::UObject** addr,
                               unrealsdk::unreal::UClass* property_class,
                               unrealsdk::unreal::UClass* meta_class)
    : ObjectComponent(std::move(name), addr, property_class), meta_class(meta_class) {}

void ClassComponent::try_set_to_object(UObject* obj) {
    if (this->property_class.is_ffield()) {
        // Shouldn't be possible, the constructor only takes uobjects
        this->cached_obj.fail_to_set(std::format("Cannot assign to an FField pointer!"));
        return;
    }
    auto prop_class = this->property_class.as_uobject();

    if (obj != nullptr && !obj->is_instance(prop_class)) {
        this->cached_obj.fail_to_set(std::format("Object is not an instance of {}:\n{}",
                                                 prop_class->Name(), obj->get_path_name()));
        return;
    }
    if (obj != nullptr && !reinterpret_cast<UClass*>(obj)->inherits(this->meta_class)) {
        this->cached_obj.fail_to_set(std::format("Object is not a subclass of {}:\n{}",
                                                 this->meta_class->Name(), obj->get_path_name()));
        return;
    }

    *reinterpret_cast<UObject**>(this->addr) = obj;
}

}  // namespace live_object_explorer
