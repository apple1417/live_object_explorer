#include "pch.h"
#include "components/object_component.h"
#include "components/abstract.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

ObjectComponent::ObjectComponent(std::string&& name, UObject** addr, UClass* property_class)
    : AbstractComponent(std::move(name)),
      cached_obj_name(NULL_OBJECT_NAME),
      addr(reinterpret_cast<uintptr_t*>(addr)),
      cached_obj(0),
      property_class(property_class) {}

void ObjectComponent::draw(const ObjectWindowSettings& /*settings*/) {
    auto current_obj = *this->addr;
    if (this->cached_obj != current_obj) {
        this->cached_obj_name = current_obj == 0
                                    ? NULL_OBJECT_NAME
                                    : this->cached_obj_name = unrealsdk::utils::narrow(
                                          reinterpret_cast<UObject*>(current_obj)->get_path_name());
        this->cached_obj = current_obj;
    }

    // TODO: proper editing
    ImGui::InputText(this->name.c_str(), this->cached_obj_name.data(),
                     this->cached_obj_name.size() + 1, ImGuiInputTextFlags_ReadOnly);
}

}  // namespace live_object_explorer
