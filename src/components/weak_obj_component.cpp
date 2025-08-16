#include "pch.h"
#include "components/weak_obj_component.h"
#include "components/object_component.h"
#include "gui.h"
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
      cached_obj_name(ObjectComponent::NULL_OBJECT_NAME) {}

void WeakObjectComponent::draw(const ObjectWindowSettings& /*settings*/,
                               ForceExpandTree /*expand_children*/,
                               bool /*show_all_children*/) {
    auto current_obj = unrealsdk::gobjects().get_weak_object(this->addr);

    if (this->cached_obj != reinterpret_cast<uintptr_t>(current_obj)) {
        this->cached_obj = reinterpret_cast<uintptr_t>(current_obj);
        this->cached_obj_name =
            (current_obj == nullptr)
                ? ObjectComponent::NULL_OBJECT_NAME
                : std::format("{}'{}'{}", current_obj->Class()->Name(),
                              unrealsdk::utils::narrow(current_obj->get_path_name()),
                              std::string_view{this->name}.substr(this->length_before_hash));
    }

    // TODO: editable
    ImGui::Text("%s:", this->hashless_name.c_str());
    ImGui::SameLine();
    if (current_obj == nullptr) {
        ImGui::TextDisabled("%s", this->cached_obj_name.c_str());
    } else {
        if (ImGui::TextLink(this->cached_obj_name.c_str())) {
            gui::open_object_window(current_obj, this->name);
        }
    }
}

[[nodiscard]] bool WeakObjectComponent::passes_filter(const ImGuiTextFilter& filter) {
    return AbstractComponent::passes_filter(filter)
           || filter.PassFilter(this->cached_obj_name.c_str());
}

}  // namespace live_object_explorer
