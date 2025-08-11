#include "pch.h"
#include "components/enum_component.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

namespace {

const std::string UNKNOWN_ENUM_PREVIEW = "Unknown";
const std::string FLAGS_PREVIEW = "(flags)";

template <typename T>
const char* pick_preview(T value,
                         const std::vector<EnumNameInfo<T>>& name_info,
                         bool flags,
                         const ObjectWindowSettings& settings) {
    if (flags) {
        return FLAGS_PREVIEW.c_str();
    }

    auto preview_info =
        std::ranges::find_if(name_info, [value](auto& info) { return info.value == value; });
    if (preview_info == name_info.end()) {
        return UNKNOWN_ENUM_PREVIEW.c_str();
    }

    return settings.hex ? preview_info->hex.c_str() : preview_info->decimal.c_str();
}

template <typename T>
void draw_flags_entry(const char* label,
                      T value,
                      T* addr,
                      const EnumNameInfo<T>& info,
                      const ObjectWindowSettings& settings) {
    bool checked = (value & info.value) == info.value;
    if (ImGui::Checkbox(label, &checked) && settings.editable) {
        if (checked) {
            *addr |= info.value;
        } else {
            *addr &= ~info.value;
        }
    }
}

template <typename T>
void draw_enum_entry(const char* label,
                     T value,
                     T* addr,
                     const EnumNameInfo<T>& info,
                     const ObjectWindowSettings& settings) {
    if (ImGui::Selectable(label, value == info.value) && settings.editable) {
        *addr = info.value;
    }
}

template <typename T>
void draw_enum(const std::string& name,
               T* addr,
               const std::vector<EnumNameInfo<T>>& name_info,
               const char*& preview,
               bool& flags,
               const ObjectWindowSettings& settings,
               ImGuiDataType data_type) {
    auto value = *addr;
    preview = pick_preview(value, name_info, flags, settings);

    if (ImGui::BeginCombo(name.c_str(), preview)) {
        ImGui::Checkbox("Display As Flags", &flags);

        ImGui::BeginDisabled(!settings.editable);
        for (const auto& info : name_info) {
            auto label = settings.hex ? info.hex.c_str() : info.decimal.c_str();

            if (flags) {
                draw_flags_entry(label, value, addr, info, settings);
            } else {
                draw_enum_entry(label, value, addr, info, settings);
            }
        }
        ImGui::EndDisabled();

        ImGui::EndCombo();
    }

    ImGui::PushID(name.c_str());
    T step = 1;
    ImGui::InputScalar("##enum_input", data_type, addr, &step, nullptr,
                       (std::is_integral_v<T> && settings.hex) ? "%X" : nullptr,
                       settings.editable ? 0 : ImGuiInputTextFlags_ReadOnly);
    ImGui::PopID();
}

}  // namespace

template <>
void Int8EnumComponent::draw(const ObjectWindowSettings& settings) {
    draw_enum(this->name, this->addr, this->name_info, this->preview, this->flags, settings,
              ImGuiDataType_S8);
}
template <>
void Int16EnumComponent::draw(const ObjectWindowSettings& settings) {
    draw_enum(this->name, this->addr, this->name_info, this->preview, this->flags, settings,
              ImGuiDataType_S16);
}
template <>
void Int32EnumComponent::draw(const ObjectWindowSettings& settings) {
    draw_enum(this->name, this->addr, this->name_info, this->preview, this->flags, settings,
              ImGuiDataType_S32);
}
template <>
void Int64EnumComponent::draw(const ObjectWindowSettings& settings) {
    draw_enum(this->name, this->addr, this->name_info, this->preview, this->flags, settings,
              ImGuiDataType_S64);
}
template <>
void UInt8EnumComponent::draw(const ObjectWindowSettings& settings) {
    draw_enum(this->name, this->addr, this->name_info, this->preview, this->flags, settings,
              ImGuiDataType_U8);
}
template <>
void UInt16EnumComponent::draw(const ObjectWindowSettings& settings) {
    draw_enum(this->name, this->addr, this->name_info, this->preview, this->flags, settings,
              ImGuiDataType_U16);
}
template <>
void UInt32EnumComponent::draw(const ObjectWindowSettings& settings) {
    draw_enum(this->name, this->addr, this->name_info, this->preview, this->flags, settings,
              ImGuiDataType_U32);
}
template <>
void UInt64EnumComponent::draw(const ObjectWindowSettings& settings) {
    draw_enum(this->name, this->addr, this->name_info, this->preview, this->flags, settings,
              ImGuiDataType_U64);
}

}  // namespace live_object_explorer
