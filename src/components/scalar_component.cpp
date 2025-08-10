#include "pch.h"
#include "components/scalar_component.h"
#include "object_window.h"

namespace live_object_explorer {

namespace {

template <typename T>
void draw_scalar(const std::string& name,
                 T* addr,
                 const ObjectWindowSettings& settings,
                 ImGuiDataType data_type) {
    T step = 1;
    ImGui::InputScalar(name.c_str(), data_type, addr, &step, nullptr,
                       (std::is_integral_v<T> && settings.hex) ? "%X" : nullptr,
                       settings.editable ? 0 : ImGuiInputTextFlags_ReadOnly);
}

}  // namespace

template <>
void Int8Component::draw(const ObjectWindowSettings& settings) {
    draw_scalar(this->name, this->addr, settings, ImGuiDataType_S8);
}
template <>
void Int16Component::draw(const ObjectWindowSettings& settings) {
    draw_scalar(this->name, this->addr, settings, ImGuiDataType_S16);
}
template <>
void Int32Component::draw(const ObjectWindowSettings& settings) {
    draw_scalar(this->name, this->addr, settings, ImGuiDataType_S32);
}
template <>
void Int64Component::draw(const ObjectWindowSettings& settings) {
    draw_scalar(this->name, this->addr, settings, ImGuiDataType_S64);
}
template <>
void UInt8Component::draw(const ObjectWindowSettings& settings) {
    draw_scalar(this->name, this->addr, settings, ImGuiDataType_U8);
}
template <>
void UInt16Component::draw(const ObjectWindowSettings& settings) {
    draw_scalar(this->name, this->addr, settings, ImGuiDataType_U16);
}
template <>
void UInt32Component::draw(const ObjectWindowSettings& settings) {
    draw_scalar(this->name, this->addr, settings, ImGuiDataType_U32);
}
template <>
void UInt64Component::draw(const ObjectWindowSettings& settings) {
    draw_scalar(this->name, this->addr, settings, ImGuiDataType_U64);
}
template <>
void FloatComponent::draw(const ObjectWindowSettings& settings) {
    draw_scalar(this->name, this->addr, settings, ImGuiDataType_Float);
}
template <>
void DoubleComponent::draw(const ObjectWindowSettings& settings) {
    draw_scalar(this->name, this->addr, settings, ImGuiDataType_Double);
}

}  // namespace live_object_explorer
