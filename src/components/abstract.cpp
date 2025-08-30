#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

AbstractComponent::AbstractComponent(std::string&& name) : name(std::move(name)) {}

bool AbstractComponent::passes_filter(const ImGuiTextFilter& filter) {
    return filter.PassFilter(this->name.c_str());
}

}  // namespace live_object_explorer
