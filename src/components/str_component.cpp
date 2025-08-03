#include "pch.h"
#include "components/str_component.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

StrComponent::StrComponent(std::string&& name, UnmanagedFString* addr)
    : AbstractComponent(std::move(name)), addr(addr) {}

void StrComponent::draw(const ObjectWindowSettings& /*settings*/) {
    // Don't have much of a choice other than converting this each tick, we might not catch
    // modifications otherwise
    std::string str = *this->addr;

    // TODO: editable
    ImGui::InputText(this->name.c_str(), str.data(), str.capacity() + 1,
                     ImGuiInputTextFlags_ReadOnly);
}

}  // namespace live_object_explorer
