#include "pch.h"
#include "components/object_field_component.h"
#include "gui/gui.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

ObjectFieldComponent::ObjectFieldComponent(std::string&& name, UObject* obj)
    : AbstractComponent(std::move(name)),
      ptr(obj),
      hashless_name(this->name.substr(0, this->length_before_hash)),
      cached_obj_name(std::format("{}'{}'{}",
                                  obj->Class()->Name(),
                                  unrealsdk::utils::narrow(obj->get_path_name()),
                                  std::string_view{this->name}.substr(this->length_before_hash))) {}

void ObjectFieldComponent::draw(const ObjectWindowSettings& /*settings*/,
                                ForceExpandTree /*expand_children*/,
                                bool /*show_all_children*/) {
    ImGui::Text("%s:", this->hashless_name.c_str());
    ImGui::SameLine();
    if (!this->ptr) {
        ImGui::TextDisabled("%s", this->cached_obj_name.c_str());
    } else {
        if (ImGui::TextLink(this->cached_obj_name.c_str())) {
            gui::open_object_window(*this->ptr, this->name);
        }
    }
}

bool ObjectFieldComponent::passes_filter(const ImGuiTextFilter& filter) {
    return AbstractComponent::passes_filter(filter)
           || filter.PassFilter(this->cached_obj_name.c_str());
}

}  // namespace live_object_explorer
