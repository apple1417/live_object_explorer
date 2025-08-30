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

void WeakObjectComponent::draw(const ObjectWindowSettings& settings,
                               ForceExpandTree /*expand_children*/,
                               bool /*show_all_children*/) {
    auto current_obj = unrealsdk::gobjects().get_weak_object(this->addr);

    if (settings.editable) {
        this->cached_obj.draw_editable(current_obj, this->name, [this](UObject* obj) {
            if (obj != nullptr && !obj->is_instance(this->property_class)) {
                this->cached_obj.fail_to_set(std::format("Object is not an instance of {}:\n{}",
                                                         this->property_class->Name(),
                                                         obj->get_path_name()));
                return;
            }

            unrealsdk::gobjects().set_weak_object(this->addr, obj);
        });
    } else {
        ImGui::Text("%s:", this->hashless_name.c_str());
        ImGui::SameLine();
        this->cached_obj.draw(current_obj, this->name);
    }
}

[[nodiscard]] bool WeakObjectComponent::passes_filter(const ImGuiTextFilter& filter) {
    return AbstractComponent::passes_filter(filter) || this->cached_obj.passes_filter(filter);
}

}  // namespace live_object_explorer
