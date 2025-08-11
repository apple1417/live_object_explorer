#ifndef COMPONENTS_SCALAR_COMPONENT_H
#define COMPONENTS_SCALAR_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

template <typename T>
class ScalarComponent : public AbstractComponent {
   protected:
    T* addr;

   public:
    /**
     * @brief Creates a new component pointing at an integral.
     *
     * @tparam T the integral type this is wrapping.
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the value being displayed.
     */
    ScalarComponent(std::string&& name, T* addr) : AbstractComponent(std::move(name)), addr(addr) {}

    ~ScalarComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
};

using Int8Component = ScalarComponent<int8_t>;
using Int16Component = ScalarComponent<int16_t>;
using Int32Component = ScalarComponent<int32_t>;
using Int64Component = ScalarComponent<int64_t>;
using UInt8Component = ScalarComponent<uint8_t>;
using UInt16Component = ScalarComponent<uint16_t>;
using UInt32Component = ScalarComponent<uint32_t>;
using UInt64Component = ScalarComponent<uint64_t>;
using FloatComponent = ScalarComponent<float32_t>;
using DoubleComponent = ScalarComponent<float64_t>;

template <>
void Int8Component::draw(const ObjectWindowSettings& settings,
                         ForceExpandTree expand_children,
                         bool show_all_children);
template <>
void Int16Component::draw(const ObjectWindowSettings& settings,
                          ForceExpandTree expand_children,
                          bool show_all_children);
template <>
void Int32Component::draw(const ObjectWindowSettings& settings,
                          ForceExpandTree expand_children,
                          bool show_all_children);
template <>
void Int64Component::draw(const ObjectWindowSettings& settings,
                          ForceExpandTree expand_children,
                          bool show_all_children);
template <>
void UInt8Component::draw(const ObjectWindowSettings& settings,
                          ForceExpandTree expand_children,
                          bool show_all_children);
template <>
void UInt16Component::draw(const ObjectWindowSettings& settings,
                           ForceExpandTree expand_children,
                           bool show_all_children);
template <>
void UInt32Component::draw(const ObjectWindowSettings& settings,
                           ForceExpandTree expand_children,
                           bool show_all_children);
template <>
void UInt64Component::draw(const ObjectWindowSettings& settings,
                           ForceExpandTree expand_children,
                           bool show_all_children);
template <>
void FloatComponent::draw(const ObjectWindowSettings& settings,
                          ForceExpandTree expand_children,
                          bool show_all_children);
template <>
void DoubleComponent::draw(const ObjectWindowSettings& settings,
                           ForceExpandTree expand_children,
                           bool show_all_children);

}  // namespace live_object_explorer

#endif /* COMPONENTS_SCALAR_COMPONENT_H */
