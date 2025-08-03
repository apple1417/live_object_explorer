#include "pch.h"
#include "components/object_component.h"
#include "components/abstract.h"
#include "gui.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

ObjectComponent::ObjectComponent(std::string&& name, UObject** addr, UClass* property_class)
    : AbstractComponent(std::move(name)),
      hashless_name(this->name.substr(0, this->length_before_hash)),
      cached_obj_name(NULL_OBJECT_NAME),
      addr(reinterpret_cast<uintptr_t*>(addr)),
      cached_obj(0),
      property_class(property_class) {}

bool ObjectComponent::may_set_to_object(UObject* obj) const {
    return obj->Class()->is_instance(this->property_class);
}

void ObjectComponent::draw(const ObjectWindowSettings& /*settings*/) {
    auto current_obj = *this->addr;
    if (this->cached_obj != current_obj) {
        this->cached_obj_name =
            current_obj == 0
                ? NULL_OBJECT_NAME
                : this->cached_obj_name = std::format(
                      "{}'{}'", reinterpret_cast<UObject*>(current_obj)->Class()->Name(),
                      unrealsdk::utils::narrow(
                          reinterpret_cast<UObject*>(current_obj)->get_path_name()));

        this->cached_obj = current_obj;
    }

    // TODO: editable
    ImGui::Text("%s:", this->hashless_name.c_str());
    ImGui::SameLine();
    if (this->cached_obj == 0) {
        ImGui::TextDisabled("%s", this->cached_obj_name.c_str());
    } else {
        if (ImGui::TextLink(this->cached_obj_name.c_str())) {
            gui::open_object_window(reinterpret_cast<UObject*>(this->cached_obj), this->name);
        }
    }
}

ClassComponent::ClassComponent(std::string&& name,
                               unrealsdk::unreal::UObject** addr,
                               unrealsdk::unreal::UClass* property_class,
                               unrealsdk::unreal::UClass* meta_class)
    : ObjectComponent(std::move(name), addr, property_class), meta_class(meta_class) {}

bool ClassComponent::may_set_to_object(UObject* obj) const {
    return ObjectComponent::may_set_to_object(obj)
           && reinterpret_cast<UClass*>(obj)->inherits(this->meta_class);
}

}  // namespace live_object_explorer
