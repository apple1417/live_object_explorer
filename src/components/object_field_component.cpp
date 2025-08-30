#include "pch.h"
#include "components/object_field_component.h"
#include "object_link.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

ObjectFieldComponent::ObjectFieldComponent(std::string&& name, UObject* obj)
    : AbstractComponent(std::move(name)), ptr(obj), cached_obj_name(format_object_name(obj)) {}

void ObjectFieldComponent::draw(const ObjectWindowSettings& /*settings*/,
                                ForceExpandTree /*expand_children*/,
                                bool /*show_all_children*/) {
    ImGui::TextUnformatted(this->name.c_str());
    ImGui::TableNextColumn();
    object_link(this->cached_obj_name, *this->ptr);
}

bool ObjectFieldComponent::passes_filter(const ImGuiTextFilter& filter) {
    return AbstractComponent::passes_filter(filter)
           || filter.PassFilter(this->cached_obj_name.c_str());
}

}  // namespace live_object_explorer
