#include "pch.h"
#include "components/unknown_component.h"
#include "components/abstract.h"
#include "object_window.h"

namespace live_object_explorer {

UnknownComponent::UnknownComponent(std::string&& name, std::string_view cls_name)
    : AbstractComponent(std::move(name)),
      msg(std::format("{}: unrecognized field type {}",
                      std::string_view{this->name}.substr(0, this->length_before_hash),
                      cls_name)) {}

void UnknownComponent::draw(const ObjectWindowSettings& settings,
                            ForceExpandTree /*expand_children*/,
                            bool /*show_all_children*/) {
    if (!settings.include_fields) {
        return;
    }
    ImGui::TextDisabled("%s", this->msg.c_str());
}

UnknownPropertyComponent::UnknownPropertyComponent(std::string&& name, std::string_view cls_name)
    : AbstractComponent(std::move(name)),
      msg(std::format("{}: unrecognized property type {}",
                      std::string_view{this->name}.substr(0, this->length_before_hash),
                      cls_name)) {}

void UnknownPropertyComponent::draw(const ObjectWindowSettings& /*settings*/,
                                    ForceExpandTree /*expand_children*/,
                                    bool /*show_all_children*/) {
    ImGui::TextDisabled("%s", this->msg.c_str());
}

}  // namespace live_object_explorer
