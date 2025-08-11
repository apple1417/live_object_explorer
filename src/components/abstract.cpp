#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

AbstractComponent::AbstractComponent(std::string&& name)
    : name(std::move(name)),
      length_before_hash(std::min(this->name.rfind("##"), this->name.size())) {}

bool AbstractComponent::passes_filter(const ImGuiTextFilter& filter) {
    return filter.PassFilter(&this->name.c_str()[0], &this->name.c_str()[this->length_before_hash]);
}

}  // namespace live_object_explorer
