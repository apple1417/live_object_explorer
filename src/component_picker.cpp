#include "pch.h"
#include "component_picker.h"
#include "components/array_component.h"
#include "components/bool_component.h"
#include "components/const_component.h"
#include "components/delegate_component.h"
#include "components/enum_component.h"
#include "components/enum_field_component.h"
#include "components/multicast_delegate_component.h"
#include "components/name_component.h"
#include "components/object_component.h"
#include "components/object_field_component.h"
#include "components/persistent_obj_ptr_component.h"
#include "components/scalar_component.h"
#include "components/str_component.h"
#include "components/struct_component.h"
#include "components/struct_field_component.h"
#include "components/weak_obj_component.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

namespace {

// ===================
// Fallback components
// ===================

/**
 * @brief Adds a fallback component for any unknown non-ZProperty - generally fields, hence the
 * name.
 *
 * @tparam T The type of the field. May be void to deliberately call the fallback.
 * @param components The list of components to add to.
 * @param field The field to add.
 * @param name The name to use for this component.
 */
template <typename T>
    requires std::negation_v<std::is_base_of<ZProperty, T>>
void insert_field_fallback(std::vector<std::unique_ptr<AbstractComponent>>& components,
                           T* field,
                           std::string&& name) {
    components.emplace_back(std::make_unique<ConstDisabledStrComponent>(
        std::move(name), std::format("unrecognized field type {}", field->Class()->Name())));
}

/**
 * @brief Adds a fallback component for any unknown ZProperty.
 *
 * @tparam T The type of the property. May be void to deliberately call the fallback.
 * @param components The list of components to add to.
 * @param prop The property to add.
 * @param name The name to use for this component.
 */
template <typename T>
    requires std::is_base_of_v<ZProperty, T>
void insert_property_fallback(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              T* prop,
                              std::string&& name) {
    components.emplace_back(std::make_unique<ConstDisabledStrComponent>(
        std::move(name), std::format("unrecognized property type {}", prop->Class()->Name())));
}

// =========================================================================
// Base components. Ideally these never fire, in favour of a specialization.
// =========================================================================

/**
 * @brief Adds a new component for any non-ZProperty - generally fields, hence the name.
 *
 * @tparam T The type of the field. May be void to deliberately call the fallback.
 * @param components The list of components to add to.
 * @param field The field to add.
 * @param name The name to use for this component.
 */
template <typename T>
    requires std::negation_v<std::is_base_of<ZProperty, T>>
void insert_field_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                            T* field,
                            std::string&& name) {
    insert_field_fallback(components, field, std::move(name));
}

/**
 * @brief Adds a new component for a ZProperty.
 *
 * @tparam T The type of the property. May be void to deliberately call the fallback.
 * @param components The list of components to add to.
 * @param prop The property to add.
 * @param name The name to use for this component.
 * @param addr The address of the value behind this component.
 */
template <typename T>
    requires std::is_base_of_v<ZProperty, T>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               T* prop,
                               std::string&& name,
                               uintptr_t /*addr*/) {
    insert_property_fallback(components, prop, std::move(name));
}

// ====================
// Per-class components
// ====================

#ifdef __clang__  // for clangd more than anything
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZArrayProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(std::make_unique<ArrayComponent>(
        std::move(name), reinterpret_cast<TArray<void>*>(addr), prop->Inner()));
}

template <>
void insert_field_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                            UBlueprintGeneratedClass* field,
                            std::string&& name) {
    components.emplace_back(std::make_unique<ObjectFieldComponent>(std::move(name), field));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZBoolProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(std::make_unique<BoolComponent>(
        std::move(name), reinterpret_cast<BoolComponent::field_mask_type*>(addr),
        prop->FieldMask()));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZByteAttributeProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    auto uenum = prop->Enum();
    if (uenum != nullptr) {
        components.emplace_back(std::make_unique<UInt8EnumComponent>(
            std::move(name), reinterpret_cast<uint8_t*>(addr), uenum));
    } else {
        components.emplace_back(
            std::make_unique<UInt8Component>(std::move(name), reinterpret_cast<uint8_t*>(addr)));
    }
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZByteProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    auto uenum = prop->Enum();
    if (uenum != nullptr) {
        components.emplace_back(std::make_unique<UInt8EnumComponent>(
            std::move(name), reinterpret_cast<uint8_t*>(addr), uenum));
    } else {
        components.emplace_back(
            std::make_unique<UInt8Component>(std::move(name), reinterpret_cast<uint8_t*>(addr)));
    }
}

template <>
void insert_field_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                            UClass* field,
                            std::string&& name) {
    components.emplace_back(std::make_unique<ObjectFieldComponent>(std::move(name), field));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZClassProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<ClassComponent>(std::move(name), reinterpret_cast<UObject**>(addr),
                                         prop->PropertyClass(), prop->MetaClass()));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZComponentProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(std::make_unique<ObjectComponent>(
        std::move(name), reinterpret_cast<UObject**>(addr), prop->PropertyClass()));
}

template <>
void insert_field_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                            UConst* field,
                            std::string&& name) {
    components.emplace_back(std::make_unique<ConstTextComponent>(std::move(name), field->Value()));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZDelegateProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(std::make_unique<DelegateComponent>(
        std::move(name), reinterpret_cast<FScriptDelegate*>(addr), prop->Signature()));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZDoubleProperty* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<DoubleComponent>(std::move(name), reinterpret_cast<float64_t*>(addr)));
}

template <>
void insert_field_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                            UEnum* field,
                            std::string&& name) {
    components.emplace_back(std::make_unique<EnumFieldComponent>(std::move(name), field));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZEnumProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    using valid_underlying_types =
        std::tuple<ZInt8Property, ZInt16Property, ZIntProperty, ZInt64Property, ZByteProperty,
                   ZUInt16Property, ZUInt32Property, ZUInt64Property>;

    auto uenum = prop->Enum();
    cast<cast_options<>::with_classes<valid_underlying_types>>(
        prop->UnderlyingProp(), [&]<typename T>(const T* /*underlying*/) {
            using data_type = PropTraits<T>::Value;
            static_assert(std::is_integral_v<data_type>);

            components.emplace_back(std::make_unique<EnumComponent<data_type>>(
                std::move(name), reinterpret_cast<data_type*>(addr), uenum));
        });
}

template <>
void insert_field_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                            UField* field,
                            std::string&& name) {
    components.emplace_back(std::make_unique<ObjectFieldComponent>(std::move(name), field));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZFloatAttributeProperty* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<FloatComponent>(std::move(name), reinterpret_cast<float32_t*>(addr)));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZFloatProperty* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<FloatComponent>(std::move(name), reinterpret_cast<float32_t*>(addr)));
}

template <>
void insert_field_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                            UFunction* field,
                            std::string&& name) {
    components.emplace_back(std::make_unique<StructFieldComponent>(std::move(name), field));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZInt16Property* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<Int16Component>(std::move(name), reinterpret_cast<int16_t*>(addr)));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZInt64Property* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<Int64Component>(std::move(name), reinterpret_cast<int64_t*>(addr)));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZInt8Property* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<Int8Component>(std::move(name), reinterpret_cast<int8_t*>(addr)));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZIntAttributeProperty* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<Int32Component>(std::move(name), reinterpret_cast<int32_t*>(addr)));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZInterfaceProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(std::make_unique<InterfaceComponent>(
        std::move(name), reinterpret_cast<UObject**>(addr), prop->InterfaceClass()));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZIntProperty* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<Int32Component>(std::move(name), reinterpret_cast<int32_t*>(addr)));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZLazyObjectProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(std::make_unique<LazyObjectComponent>(
        std::move(name), reinterpret_cast<FLazyObjectPtr*>(addr), prop->PropertyClass()));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZMulticastDelegateProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(std::make_unique<MulticastDelegateComponent>(
        std::move(name), reinterpret_cast<TArray<FScriptDelegate>*>(addr), prop->Signature()));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZNameProperty* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<NameComponent>(std::move(name), reinterpret_cast<FName*>(addr)));
}

template <>
void insert_field_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                            UObject* field,
                            std::string&& name) {
    components.emplace_back(std::make_unique<ObjectFieldComponent>(std::move(name), field));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZObjectProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(std::make_unique<ObjectComponent>(
        std::move(name), reinterpret_cast<UObject**>(addr), prop->PropertyClass()));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZProperty* prop,
                               std::string&& name,
                               uintptr_t /*addr*/) {
    // Just forward directly to the fallback
    insert_property_fallback(components, prop, std::move(name));
}

template <>
void insert_field_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                            UScriptStruct* field,
                            std::string&& name) {
    components.emplace_back(std::make_unique<StructFieldComponent>(std::move(name), field));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZSoftClassProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(std::make_unique<SoftClassComponent>(
        std::move(name), reinterpret_cast<FSoftObjectPtr*>(addr), prop->PropertyClass(),
        prop->MetaClass()));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZSoftObjectProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(std::make_unique<SoftObjectComponent>(
        std::move(name), reinterpret_cast<FSoftObjectPtr*>(addr), prop->PropertyClass()));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZStrProperty* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<StrComponent>(std::move(name), reinterpret_cast<UnmanagedFString*>(addr)));
}

template <>
void insert_field_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                            UStruct* field,
                            std::string&& name) {
    components.emplace_back(std::make_unique<ObjectFieldComponent>(std::move(name), field));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZStructProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<StructComponent>(std::move(name), addr, prop->Struct()));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZTextProperty* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<TextComponent>(std::move(name), reinterpret_cast<FText*>(addr)));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZUInt16Property* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<UInt16Component>(std::move(name), reinterpret_cast<uint16_t*>(addr)));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZUInt32Property* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<UInt32Component>(std::move(name), reinterpret_cast<uint32_t*>(addr)));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZUInt64Property* /*prop*/,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(
        std::make_unique<UInt64Component>(std::move(name), reinterpret_cast<uint64_t*>(addr)));
}

template <>
void insert_property_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                               ZWeakObjectProperty* prop,
                               std::string&& name,
                               uintptr_t addr) {
    components.emplace_back(std::make_unique<WeakObjectComponent>(
        std::move(name), reinterpret_cast<FWeakObjectPtr*>(addr), prop->PropertyClass()));
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

template <typename InputType>
void insert_component_impl(std::vector<std::unique_ptr<AbstractComponent>>& prop_components,
                           std::vector<std::unique_ptr<AbstractComponent>>& field_components,
                           InputType* obj,
                           uintptr_t base_addr) {
    cast<cast_options<>::with_input<true>>(
        obj,
        [&prop_components, &field_components, base_addr]<typename T>(T* obj) {
            if constexpr (std::is_base_of_v<ZProperty, T>) {
                auto offset_internal = obj->Offset_Internal();
                auto array_dim = obj->ArrayDim();
                if (array_dim > 1) {
                    auto element_size = obj->ElementSize();
                    for (decltype(array_dim) i = 0; i < array_dim; i++) {
                        auto addr = base_addr + offset_internal + (i * element_size);

                        insert_property_component<T>(prop_components, obj,
                                                     std::format("{}[{}]", obj->Name(), i), addr);
                    }
                } else {
                    auto addr = base_addr + offset_internal;

                    insert_property_component<T>(prop_components, obj,
                                                 static_cast<std::string>(obj->Name()), addr);
                }
            } else {
                insert_field_component(field_components, obj,
                                       static_cast<std::string>(obj->Name()));
            }
        },
        [&prop_components, &field_components](InputType* obj) {
            // If the cast fails, still split by property or not
            if (obj->is_instance(find_class<ZProperty>())) {
                insert_property_fallback(prop_components, reinterpret_cast<ZProperty*>(obj),
                                         static_cast<std::string>(obj->Name()));
            } else {
                insert_field_fallback(field_components, obj, static_cast<std::string>(obj->Name()));
            }
        });
}

}  // namespace

void insert_component(std::vector<std::unique_ptr<AbstractComponent>>& prop_components,
                      std::vector<std::unique_ptr<AbstractComponent>>& field_components,
                      UObject* obj,
                      uintptr_t base_addr) {
    insert_component_impl(prop_components, field_components, obj, base_addr);
}
void insert_component(std::vector<std::unique_ptr<AbstractComponent>>& prop_components,
                      std::vector<std::unique_ptr<AbstractComponent>>& field_components,
                      FField* field,
                      uintptr_t base_addr) {
    insert_component_impl(prop_components, field_components, field, base_addr);
}

void insert_component_array(std::vector<std::unique_ptr<AbstractComponent>>& prop_components,
                            unrealsdk::unreal::TArray<void>* arr,
                            unrealsdk::unreal::ZProperty* inner_prop,
                            size_t idx) {
    cast<cast_options<>::with_input<true>>(
        inner_prop,
        [&prop_components, arr, idx]<typename T>(T* inner_prop) {
            auto addr = reinterpret_cast<uintptr_t>(arr->data) + (idx * inner_prop->ElementSize());

            insert_property_component<T>(prop_components, inner_prop, std::format("[{}]", idx),
                                         addr);
        },
        [&prop_components, idx](ZProperty* inner_prop) {
            insert_property_fallback(prop_components, inner_prop, std::format("[{}]", idx));
        });
}

}  // namespace live_object_explorer
