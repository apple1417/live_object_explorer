#include "pch.h"
#include "components/unknown_component.h"
#include "object_window.h"

namespace live_object_explorer {

UnknownComponent::UnknownComponent(std::string_view name, std::string_view cls_name)
    : name(name), cls_name(cls_name) {}

void UnknownComponent::draw(const ObjectWindowSettings& /*settings*/) {
    ImGui::TextDisabled("%s: unrecognized property type %s", this->name.c_str(),
                        this->cls_name.c_str());
}

}  // namespace live_object_explorer
