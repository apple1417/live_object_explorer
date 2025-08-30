#include "pch.h"
#include "components/weak_obj_component.h"
#include "object_link.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

WeakObjectComponent::WeakObjectComponent(std::string&& name,
                                         unrealsdk::unreal::FWeakObjectPtr* addr,
                                         unrealsdk::unreal::UClass* property_class)
    : AbstractComponent(std::move(name)),
      hashless_name(this->name.substr(0, this->length_before_hash)),
      addr(addr),
      property_class(property_class),
      cached_obj(std::string_view{this->name}.substr(this->length_before_hash)) {}

void WeakObjectComponent::draw(const ObjectWindowSettings& /*settings*/,
                               ForceExpandTree /*expand_children*/,
                               bool /*show_all_children*/) {
    auto current_obj = unrealsdk::gobjects().get_weak_object(this->addr);

    // TODO: editable
    ImGui::Text("%s:", this->hashless_name.c_str());
    ImGui::SameLine();
    this->cached_obj.draw(current_obj, this->name);
}

[[nodiscard]] bool WeakObjectComponent::passes_filter(const ImGuiTextFilter& filter) {
    return AbstractComponent::passes_filter(filter) || this->cached_obj.passes_filter(filter);
}

}  // namespace live_object_explorer
