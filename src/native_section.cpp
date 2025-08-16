#include "pch.h"
#include "native_section.h"
#include "components/enum_field_component.h"
#include "components/name_component.h"
#include "components/object_component.h"
#include "components/object_field_component.h"
#include "components/scalar_component.h"
#include "components/str_component.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

namespace {

/**
 * @brief Helper to append an object pointer, automatically deducing the relevant type.
 *
 * @tparam T The object type to append.
 * @param components The list of components to append to.
 * @param name The name of the component.
 * @param obj Pointer to the object pointer to append.
 */
template <typename T>
    requires std::is_base_of_v<UObject, T>
void append_object_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                             std::string&& name,
                             T** obj) {
    components.emplace_back(std::make_unique<ObjectComponent>(
        std::move(name), reinterpret_cast<UObject**>(obj), find_class<T>()));
}

/**
 * @brief Helper to append a scalar component, automatically deducing the relevant type.
 *
 * @tparam T The scalar's type.
 * @param components The list of components to append to.
 * @param name The name of the component.
 * @param val Pointer to the value to append.
 */
template <typename T>
void append_scalar_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                             std::string&& name,
                             T* val) {
    components.emplace_back(std::make_unique<ScalarComponent<T>>(std::move(name), val));
}

/**
 * @brief Recursively inserts all native components for the templated class + parent classes.
 *
 * @tparam T The type of the current object.
 * @param components The list of components to add to.
 * @param obj The object to gather native components of.
 */
template <typename T>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components, T* obj);

#ifdef __clang__  // for clangd more than anything
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

// =================================================================================================

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UObject* obj) {
    append_scalar_component(components, "ObjectFlags", &obj->ObjectFlags());
    append_scalar_component(components, "InternalIndex", &obj->InternalIndex());
    append_object_component(components, "Class", &obj->Class());
    components.emplace_back(std::make_unique<NameComponent>("Name", &obj->Name()));
    append_object_component(components, "Outer", &obj->Outer());
}

// =================================================================================================

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UField* obj) {
    append_object_component(components, "Next", &obj->Next());

    insert_native_components<UObject>(components, obj);
}

// =================================================================================================

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UConst* obj) {
    components.emplace_back(std::make_unique<StrComponent>("Value", &obj->Value()));

    insert_native_components<UField>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UEnum* obj) {
    // TODO: technically we should have a custom component type if we want this to be editable
    components.emplace_back(std::make_unique<EnumFieldComponent>("Names", obj));

    insert_native_components<UField>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UProperty* obj) {
    append_scalar_component(components, "ArrayDim", &obj->ArrayDim());
    append_scalar_component(components, "ElementSize", &obj->ElementSize());
    append_scalar_component(components, "PropertyFlags", &obj->PropertyFlags());
    append_scalar_component(components, "Offset_Internal", &obj->Offset_Internal());
    append_object_component(components, "PropertyLinkNext", &obj->PropertyLinkNext());

    insert_native_components<UField>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UStruct* obj) {
    append_object_component(components, "SuperField", &obj->SuperField());
    append_object_component(components, "Children", &obj->Children());
    append_scalar_component(components, "PropertySize", &obj->PropertySize());
    append_object_component(components, "PropertyLink", &obj->PropertyLink());

    insert_native_components<UField>(components, obj);
}

// =================================================================================================

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UArrayProperty* obj) {
    append_object_component(components, "Inner", &obj->Inner());

    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UBoolProperty* obj) {
    append_scalar_component(components, "FieldMask", &obj->FieldMask());

    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UByteProperty* obj) {
    append_object_component(components, "Enum", &obj->Enum());

    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UClass* obj) {
    append_object_component(components, "ClassDefaultObject", &obj->ClassDefaultObject());
    // TODO: `Interfaces` needs its own component type

    insert_native_components<UStruct>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UDelegateProperty* obj) {
    append_object_component(components, "Signature", &obj->Signature());

    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UDoubleProperty* obj) {
    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UEnumProperty* obj) {
    append_object_component(components, "UnderlyingProp", &obj->UnderlyingProp());
    append_object_component(components, "Enum", &obj->Enum());

    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UFloatProperty* obj) {
    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UFunction* obj) {
    append_scalar_component(components, "FunctionFlags", &obj->FunctionFlags());
    append_scalar_component(components, "NumParams", &obj->NumParams());
    append_scalar_component(components, "ParamsSize", &obj->ParamsSize());
    append_scalar_component(components, "ReturnValueOffset", &obj->ReturnValueOffset());

    insert_native_components<UStruct>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UInt16Property* obj) {
    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UInt64Property* obj) {
    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UInt8Property* obj) {
    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UInterfaceProperty* obj) {
    append_object_component(components, "InterfaceClass", &obj->InterfaceClass());

    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UIntProperty* obj) {
    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UMulticastDelegateProperty* obj) {
    append_object_component(components, "Signature", &obj->Signature());

    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UNameProperty* obj) {
    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UObjectProperty* obj) {
    append_object_component(components, "PropertyClass", &obj->PropertyClass());

    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UScriptStruct* obj) {
    append_scalar_component(components, "StructFlags", &obj->StructFlags());

    insert_native_components<UStruct>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UStrProperty* obj) {
    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UStructProperty* obj) {
    append_object_component(components, "Struct", &obj->Struct());

    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UTextProperty* obj) {
    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UUInt16Property* obj) {
    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UUInt32Property* obj) {
    insert_native_components<UProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UUInt64Property* obj) {
    insert_native_components<UProperty>(components, obj);
}

// =================================================================================================

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UBlueprintGeneratedClass* obj) {
    insert_native_components<UClass>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UByteAttributeProperty* obj) {
    append_object_component(components, "ModifierStackProperty", &obj->ModifierStackProperty());
    append_object_component(components, "OtherAttributeProperty", &obj->OtherAttributeProperty());

    insert_native_components<UByteProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UClassProperty* obj) {
    append_object_component(components, "MetaClass", &obj->MetaClass());

    insert_native_components<UObjectProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UComponentProperty* obj) {
    insert_native_components<UObjectProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UFloatAttributeProperty* obj) {
    append_object_component(components, "ModifierStackProperty", &obj->ModifierStackProperty());
    append_object_component(components, "OtherAttributeProperty", &obj->OtherAttributeProperty());

    insert_native_components<UFloatProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UIntAttributeProperty* obj) {
    append_object_component(components, "ModifierStackProperty", &obj->ModifierStackProperty());
    append_object_component(components, "OtherAttributeProperty", &obj->OtherAttributeProperty());

    insert_native_components<UIntProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              ULazyObjectProperty* obj) {
    insert_native_components<UObjectProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              USoftObjectProperty* obj) {
    insert_native_components<UObjectProperty>(components, obj);
}

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              UWeakObjectProperty* obj) {
    insert_native_components<UObjectProperty>(components, obj);
}

// =================================================================================================

template <>
void insert_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                              USoftClassProperty* obj) {
    append_object_component(components, "MetaClass", &obj->MetaClass());

    insert_native_components<USoftObjectProperty>(components, obj);
}

// =================================================================================================

#ifdef __clang__
#pragma clang diagnostic pop
#endif

}  // namespace

void insert_all_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                                  unrealsdk::unreal::UObject* obj) {
    cast<cast_options<true, true>>(
        obj, [&components]<typename T>(T* obj) { insert_native_components(components, obj); });
}

}  // namespace live_object_explorer
