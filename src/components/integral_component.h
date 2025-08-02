#ifndef COMPONENTS_INTEGRAL_COMPONENT_H
#define COMPONENTS_INTEGRAL_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

template <std::integral T>
class IntegralComponent : public AbstractComponent {
   private:
    std::string name;
    T* addr;

   public:
    /**
     * @brief Creates a new component pointing at an integral.
     *
     * @tparam T the integral type this is wrapping.
     * @param name The name of the property this is wrapping.
     * @param addr Pointer to the value being displayed.
     */
    IntegralComponent(std::string_view name, T* addr) : name(name), addr(addr) {}

    ~IntegralComponent() override = default;
    void draw(const ObjectWindowSettings& settings) override;
};

using Int8Component = IntegralComponent<int8_t>;
using Int16Component = IntegralComponent<int16_t>;
using IntComponent = IntegralComponent<int32_t>;
using Int64Component = IntegralComponent<int64_t>;
using UInt16Component = IntegralComponent<uint16_t>;
using UInt32Component = IntegralComponent<uint32_t>;
using UInt64Component = IntegralComponent<uint64_t>;

// TODO: byte component is different because it includes an enum

template <>
void Int8Component::draw(const ObjectWindowSettings& settings);
template <>
void Int16Component::draw(const ObjectWindowSettings& settings);
template <>
void IntComponent::draw(const ObjectWindowSettings& settings);
template <>
void Int64Component::draw(const ObjectWindowSettings& settings);
template <>
void UInt16Component::draw(const ObjectWindowSettings& settings);
template <>
void UInt32Component::draw(const ObjectWindowSettings& settings);
template <>
void UInt64Component::draw(const ObjectWindowSettings& settings);

}  // namespace live_object_explorer

#endif /* COMPONENTS_INTEGRAL_COMPONENT_H */
