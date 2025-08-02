#include "pch.h"
#include "components/object_component.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

ObjectComponent::ObjectComponent(std::string_view name, UObject** addr)
    : name(name), addr(reinterpret_cast<uintptr_t*>(addr)) {}

void ObjectComponent::draw(const ObjectWindowSettings& /*settings*/) {
    auto current_obj = *this->addr;
    if (this->cached_obj != current_obj) {
        this->cached_obj_name = current_obj == 0
                                    ? "null"
                                    : this->cached_obj_name = unrealsdk::utils::narrow(
                                          reinterpret_cast<UObject*>(current_obj)->get_path_name());
        this->cached_obj = current_obj;
    }

    // TODO: proper editing
    ImGui::InputText(this->name.c_str(), this->cached_obj_name.data(),
                     this->cached_obj_name.size() + 1, ImGuiInputTextFlags_ReadOnly);
}

}  // namespace live_object_explorer
