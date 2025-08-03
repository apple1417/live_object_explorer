#include "pch.h"
#include "component_picker.h"
#include "components/object_component.h"
#include "components/scalar_component.h"
#include "components/unknown_component.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

namespace {

#ifdef __clang__  // for clangd more than anyhting
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

/**
 * @fn insert_property_component
 * @brief Adds a new component for a UProperty.
 *
 * @param components The list of components to add to.
 * @param field The property to add.
 * @param name The name to use for this component.
 * @param addr The address of the value behind this component.
 */

/**
 * @fn insert_field_component
 * @brief Adds a new component for any non-UProperty field.
 *
 * @param components The list of components to add to.
 * @param field The field to add.
 * @param name The name to use for this component.
 */

// UArrayProperty
// UBlueprintGeneratedClass
// UBoolProperty
// UByteAttributeProperty
// UByteProperty
// UClass
// UClassProperty
// UComponentProperty
// UConst
// UDelegateProperty

void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               UDoubleProperty* /*field*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<DoubleComponent>(std::move(name), reinterpret_cast<float64_t*>(addr)));
}

// UEnum
// UEnumProperty
// UField
// UFloatAttributeProperty

void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               UFloatProperty* /*field*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<FloatComponent>(std::move(name), reinterpret_cast<float32_t*>(addr)));
}

// UFunction

void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               UInt16Property* /*field*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<Int16Component>(std::move(name), reinterpret_cast<int16_t*>(addr)));
}

void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               UInt64Property* /*field*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<Int64Component>(std::move(name), reinterpret_cast<int64_t*>(addr)));
}

void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               UInt8Property* /*field*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<Int8Component>(std::move(name), reinterpret_cast<int8_t*>(addr)));
}

// UIntAttributeProperty
// UInterfaceProperty

void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               UIntProperty* /*field*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<IntComponent>(std::move(name), reinterpret_cast<int32_t*>(addr)));
}

void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               UObjectProperty* field,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(std::make_unique<ObjectComponent>(
        std::move(name), reinterpret_cast<UObject**>(addr), field->PropertyClass()));
}

// ULazyObjectProperty
// UMulticastDelegateProperty
// UNameProperty

void insert_field_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                            UObject* field,
                            std::string&& name) {
    // Fallback to unknown
    components.emplace_back(
        std::make_unique<UnknownComponent>(std::move(name), (std::string)field->Class()->Name()));
}

// UObjectProperty

void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               UProperty* field,
                               std::string&& name,
                               uintptr_t /*addr*/) {
    // Fallback to unknown
    components.emplace_back(std::make_unique<UnknownPropertyComponent>(
        std::move(name), (std::string)field->Class()->Name()));
}

// UScriptStruct
// USoftClassProperty
// USoftObjectProperty
// UStrProperty
// UStruct
// UStructProperty
// UTextProperty

void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               UUInt16Property* /*field*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<UInt16Component>(std::move(name), reinterpret_cast<uint16_t*>(addr)));
}
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               UUInt32Property* /*field*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<UInt32Component>(std::move(name), reinterpret_cast<uint32_t*>(addr)));
}
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               UUInt64Property* /*field*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<UInt64Component>(std::move(name), reinterpret_cast<uint64_t*>(addr)));
}

// UWeakObjectProperty

#ifdef __clang__
#pragma clang diagnostic pop
#endif

}  // namespace

void insert_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                      unrealsdk::unreal::UField* field,
                      uintptr_t base_addr) {
    cast(field, [&components, base_addr]<typename T>(T* field) {
        if constexpr (std::is_base_of_v<UProperty, T>) {
            auto offset_internal = field->Offset_Internal();
            auto array_dim = field->ArrayDim();
            if (array_dim > 1) {
                auto element_size = field->ElementSize();
                for (decltype(array_dim) i = 0; i < array_dim; i++) {
                    auto name = std::format("{}[{}]##comp_{}", field->Name(), i, components.size());
                    auto addr = base_addr + offset_internal + (i * element_size);

                    insert_property_component(components, field, std::move(name), addr);
                }
            } else {
                auto name = std::format("{}##comp_{}", field->Name(), components.size());
                auto addr = base_addr + offset_internal;

                insert_property_component(components, field, std::move(name), addr);
            }
        } else {
            auto name = std::format("{}##comp_{}", field->Name(), components.size());
            insert_field_component(components, field, std::move(name));
        }
    });
}

}  // namespace live_object_explorer
