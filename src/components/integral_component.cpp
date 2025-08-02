#include "pch.h"
#include "components/integral_component.h"
#include "object_window.h"

namespace live_object_explorer {

namespace {

template <std::integral T>
void draw_scalar(const std::string& name,
                 T* addr,
                 T* step,
                 const ObjectWindowSettings& settings,
                 ImGuiDataType data_type) {
    ImGui::InputScalar(name.c_str(), data_type, addr, step, nullptr, settings.hex ? "%X" : nullptr,
                       settings.editable ? 0 : ImGuiInputTextFlags_ReadOnly);
}

}  // namespace

template <>
void Int8Component::draw(const ObjectWindowSettings& settings) {
    int8_t step = 1;
    draw_scalar(this->name, this->addr, &step, settings, ImGuiDataType_S8);
}
template <>
void Int16Component::draw(const ObjectWindowSettings& settings) {
    int16_t step = 1;
    draw_scalar(this->name, this->addr, &step, settings, ImGuiDataType_S16);
}
template <>
void IntComponent::draw(const ObjectWindowSettings& settings) {
    int32_t step = 1;
    draw_scalar(this->name, this->addr, &step, settings, ImGuiDataType_S32);
}
template <>
void Int64Component::draw(const ObjectWindowSettings& settings) {
    int64_t step = 1;
    draw_scalar(this->name, this->addr, &step, settings, ImGuiDataType_S64);
}
template <>
void UInt16Component::draw(const ObjectWindowSettings& settings) {
    uint16_t step = 1;
    draw_scalar(this->name, this->addr, &step, settings, ImGuiDataType_U16);
}
template <>
void UInt32Component::draw(const ObjectWindowSettings& settings) {
    uint32_t step = 1;
    draw_scalar(this->name, this->addr, &step, settings, ImGuiDataType_U32);
}
template <>
void UInt64Component::draw(const ObjectWindowSettings& settings) {
    uint64_t step = 1;
    draw_scalar(this->name, this->addr, &step, settings, ImGuiDataType_U64);
}

}  // namespace live_object_explorer
