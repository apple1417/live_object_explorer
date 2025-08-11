#ifndef COMPONENTS_ENUM_COMPONENT_H
#define COMPONENTS_ENUM_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

template <typename T>
struct EnumNameInfo {
    std::string decimal;
    std::string hex;
    T value;
};

template <typename T>
class EnumComponent : public AbstractComponent {
   protected:
    std::vector<EnumNameInfo<T>> name_info;
    T* addr;
    const char* preview{};
    bool flags;

   public:
    /**
     * @brief Creates a new component pointing at an enum property.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the property.
     * @param uenum The enum type being pointed at.
     */
    EnumComponent(std::string&& name, T* addr, unrealsdk::unreal::UEnum* uenum)
        : AbstractComponent(std::move(name)), addr(addr) {
        std::ranges::copy(uenum->get_names() | std::views::transform([](auto pair) {
                              return EnumNameInfo<T>{
                                  std::format("{} ({})", pair.first, pair.second),
                                  std::format("{} ({:X})", pair.first, pair.second),
                                  (T)pair.second};
                          }),
                          std::back_inserter(this->name_info));
        // NOLINTNEXTLINE(readability-identifier-length)
        std::ranges::sort(this->name_info, [](auto a, auto b) { return a.value < b.value; });

        // If there are any gaps in the values, assume a flags enum by default.
        T last_value = 0;
        for (auto info : this->name_info) {
            // Allow starting with either 0 or 1
            if (info.value == 0 && last_value == 0) {
                continue;
            }
            if (info.value != last_value + 1) {
                this->flags = true;
                return;
            }
            last_value = info.value;
        }

        this->flags = false;
    }

    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override {
        return AbstractComponent::passes_filter(filter) || filter.PassFilter(this->preview);
    }

    ~EnumComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
};

using Int8EnumComponent = EnumComponent<int8_t>;
using Int16EnumComponent = EnumComponent<int16_t>;
using Int32EnumComponent = EnumComponent<int32_t>;
using Int64EnumComponent = EnumComponent<int64_t>;
using UInt8EnumComponent = EnumComponent<uint8_t>;
using UInt16EnumComponent = EnumComponent<uint16_t>;
using UInt32EnumComponent = EnumComponent<uint32_t>;
using UInt64EnumComponent = EnumComponent<uint64_t>;

template <>
void Int8EnumComponent::draw(const ObjectWindowSettings& settings,
                             ForceExpandTree expand_children,
                             bool show_all_children);
template <>
void Int16EnumComponent::draw(const ObjectWindowSettings& settings,
                              ForceExpandTree expand_children,
                              bool show_all_children);
template <>
void Int32EnumComponent::draw(const ObjectWindowSettings& settings,
                              ForceExpandTree expand_children,
                              bool show_all_children);
template <>
void Int64EnumComponent::draw(const ObjectWindowSettings& settings,
                              ForceExpandTree expand_children,
                              bool show_all_children);
template <>
void UInt8EnumComponent::draw(const ObjectWindowSettings& settings,
                              ForceExpandTree expand_children,
                              bool show_all_children);
template <>
void UInt16EnumComponent::draw(const ObjectWindowSettings& settings,
                               ForceExpandTree expand_children,
                               bool show_all_children);
template <>
void UInt32EnumComponent::draw(const ObjectWindowSettings& settings,
                               ForceExpandTree expand_children,
                               bool show_all_children);
template <>
void UInt64EnumComponent::draw(const ObjectWindowSettings& settings,
                               ForceExpandTree expand_children,
                               bool show_all_children);

}  // namespace live_object_explorer

#endif /* COMPONENTS_ENUM_COMPONENT_H */
