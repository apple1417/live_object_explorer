#include "pch.h"
#include "refs_searcher.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer::refs {

/**
 * @brief Finds any refs in native fields on the object.
 *
 * @tparam T The type of the object.
 * @param obj The object to search form.
 * @param callback A callback to call with any discovered refs.
 */
template <typename T>
    requires std::is_base_of_v<UObject, T>
void find_native_refs(T* obj, const refs_callback& callback);

/**
 * @brief Finds any refs in fields controlled by properties.
 *
 * @tparam T The type of the property.
 * @param prop The property to look for references in.
 * @param idx The fixed array index to get the value at.
 * @param base_addr The base address of the object to read the property from.
 * @param obj The base object the search started from.
 * @param callback A callback to call with any discovered refs.
 */
template <typename T>
    requires std::is_base_of_v<ZProperty, T>
void find_property_refs(T* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback);

// =================================================================================================

template <>
void find_native_refs(UObject* obj, const refs_callback& callback) {
    callback(obj, obj->Class());
    callback(obj, obj->Outer());
}

// ======== First Layer Subclasses ========

template <>
void find_native_refs(UField* obj, const refs_callback& callback) {
    callback(obj, obj->Next());
    find_native_refs<UObject>(obj, callback);
}

// ======== Second Layer Subclasses ========

template <>
void find_native_refs(UConst* obj, const refs_callback& callback) {
    find_native_refs<UField>(obj, callback);
}

template <>
void find_native_refs(UEnum* obj, const refs_callback& callback) {
    find_native_refs<UField>(obj, callback);
}

// We can only hold UObject refs, so if properties are fields we need to exlude them here
#if !UNREALSDK_PROPERTIES_ARE_FFIELD
template <>
void find_native_refs(ZProperty* obj, const refs_callback& callback) {
    callback(obj, obj->PropertyLinkNext());
    find_native_refs<UField>(obj, callback);
}
#endif

template <>
void find_native_refs(UStruct* obj, const refs_callback& callback) {
    callback(obj, obj->SuperField());
    callback(obj, obj->Children());
#if !UNREALSDK_PROPERTIES_ARE_FFIELD
    callback(obj, obj->PropertyLink());
#endif
    find_native_refs<UField>(obj, callback);
}

// ======== Third Layer Subclasses ========

#if !UNREALSDK_PROPERTIES_ARE_FFIELD
template <>
void find_native_refs(ZArrayProperty* obj, const refs_callback& callback) {
    callback(obj, obj->Inner());
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZBoolProperty* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZByteProperty* obj, const refs_callback& callback) {
    callback(obj, obj->Enum());
    find_native_refs<ZProperty>(obj, callback);
}
#endif

template <>
void find_native_refs(UClass* obj, const refs_callback& callback) {
    callback(obj, obj->ClassDefaultObject());
    find_native_refs<UStruct>(obj, callback);
}

#if !UNREALSDK_PROPERTIES_ARE_FFIELD
template <>
void find_native_refs(ZDelegateProperty* obj, const refs_callback& callback) {
    callback(obj, obj->Signature());
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZDoubleProperty* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZEnumProperty* obj, const refs_callback& callback) {
    callback(obj, obj->UnderlyingProp());
    callback(obj, obj->Enum());
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZFloatProperty* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}
#endif

template <>
void find_native_refs(UFunction* obj, const refs_callback& callback) {
    find_native_refs<UStruct>(obj, callback);
}

#if !UNREALSDK_PROPERTIES_ARE_FFIELD
template <>
void find_native_refs(ZGameDataHandleProperty* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZGbxDefPtrProperty* obj, const refs_callback& callback) {
    callback(obj, obj->Struct());
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZInt8Property* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZInt16Property* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZInt64Property* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZInterfaceProperty* obj, const refs_callback& callback) {
    callback(obj, obj->InterfaceClass());
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZIntProperty* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZMulticastDelegateProperty* obj, const refs_callback& callback) {
    callback(obj, obj->Signature());
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZNameProperty* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZObjectProperty* obj, const refs_callback& callback) {
    callback(obj, obj->PropertyClass());
    find_native_refs<ZProperty>(obj, callback);
}
#endif

template <>
void find_native_refs(UScriptStruct* obj, const refs_callback& callback) {
    find_native_refs<UStruct>(obj, callback);
}

#if !UNREALSDK_PROPERTIES_ARE_FFIELD
template <>
void find_native_refs(ZStrProperty* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZStructProperty* obj, const refs_callback& callback) {
    callback(obj, obj->Struct());
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZTextProperty* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZUInt16Property* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZUInt32Property* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}

template <>
void find_native_refs(ZUInt64Property* obj, const refs_callback& callback) {
    find_native_refs<ZProperty>(obj, callback);
}
#endif

// ======== Fourth Layer Subclasses ========

template <>
void find_native_refs(UBlueprintGeneratedClass* obj, const refs_callback& callback) {
    find_native_refs<UClass>(obj, callback);
}

#if !UNREALSDK_PROPERTIES_ARE_FFIELD
template <>
void find_native_refs(ZByteAttributeProperty* obj, const refs_callback& callback) {
    callback(obj, obj->ModifierStackProperty());
    callback(obj, obj->OtherAttributeProperty());
    find_native_refs<ZByteProperty>(obj, callback);
}

template <>
void find_native_refs(ZClassProperty* obj, const refs_callback& callback) {
    callback(obj, obj->MetaClass());
    find_native_refs<ZObjectProperty>(obj, callback);
}

template <>
void find_native_refs(ZComponentProperty* obj, const refs_callback& callback) {
    find_native_refs<ZObjectProperty>(obj, callback);
}

template <>
void find_native_refs(ZFloatAttributeProperty* obj, const refs_callback& callback) {
    callback(obj, obj->ModifierStackProperty());
    callback(obj, obj->OtherAttributeProperty());
    find_native_refs<ZFloatProperty>(obj, callback);
}

template <>
void find_native_refs(ZGbxInlineStructProperty* obj, const refs_callback& callback) {
    callback(obj, obj->MetaStruct());
    find_native_refs<ZStructProperty>(obj, callback);
}

template <>
void find_native_refs(ZIntAttributeProperty* obj, const refs_callback& callback) {
    callback(obj, obj->ModifierStackProperty());
    callback(obj, obj->OtherAttributeProperty());
    find_native_refs<ZIntProperty>(obj, callback);
}

template <>
void find_native_refs(ZLazyObjectProperty* obj, const refs_callback& callback) {
    find_native_refs<ZObjectProperty>(obj, callback);
}

template <>
void find_native_refs(ZSoftObjectProperty* obj, const refs_callback& callback) {
    find_native_refs<ZObjectProperty>(obj, callback);
}

template <>
void find_native_refs(ZWeakObjectProperty* obj, const refs_callback& callback) {
    find_native_refs<ZObjectProperty>(obj, callback);
}
#endif

// ======== Fifth Layer Subclasses ========

#if !UNREALSDK_PROPERTIES_ARE_FFIELD
template <>
void find_native_refs(ZSoftClassProperty* obj, const refs_callback& callback) {
    find_native_refs<ZSoftObjectProperty>(obj, callback);
}
#endif

// =================================================================================================
// NOLINTBEGIN(readability-named-parameter)

// ======== Third Layer Subclasses ========

template <>
void find_property_refs(ZArrayProperty* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback) {
    auto arr = get_property(prop, idx, base_addr).base.get();
    cast(
        prop->Inner(),
        [arr, obj, &callback]<typename T>(T* inner) {
            auto element_size = inner->ElementSize();
            for (size_t i = 0; i < arr->size(); i++) {
                find_property_refs<T>(inner, 0,
                                      reinterpret_cast<uintptr_t>(arr->data) + (element_size * i),
                                      obj, callback);
            }
        },
        // Fallback: ignore this property, assume no refs
        [](ZProperty* /*prop*/) {});
}

template <>
void find_property_refs(ZBoolProperty*, size_t, uintptr_t, UObject*, const refs_callback&) {}
template <>
void find_property_refs(ZByteProperty*, size_t, uintptr_t, UObject*, const refs_callback&) {}

template <>
void find_property_refs(ZDelegateProperty* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback) {
    auto delegate = get_property(prop, idx, base_addr);
    if (delegate.has_value()) {
        callback(obj, delegate->object);
        callback(obj, delegate->func);
    }
}

template <>
void find_property_refs(ZDoubleProperty*, size_t, uintptr_t, UObject*, const refs_callback&) {}
template <>
void find_property_refs(ZEnumProperty*, size_t, uintptr_t, UObject*, const refs_callback&) {}
template <>
void find_property_refs(ZFloatProperty*, size_t, uintptr_t, UObject*, const refs_callback&) {}

template <>
void find_property_refs(ZGameDataHandleProperty*,
                        size_t,
                        uintptr_t,
                        UObject*,
                        const refs_callback&) {
    // TODO: OAK2
}

template <>
void find_property_refs(ZGbxDefPtrProperty*, size_t, uintptr_t, UObject*, const refs_callback&) {
    // TODO: OAK2
}

template <>
void find_property_refs(ZInt8Property*, size_t, uintptr_t, UObject*, const refs_callback&) {}
template <>
void find_property_refs(ZInt16Property*, size_t, uintptr_t, UObject*, const refs_callback&) {}
template <>
void find_property_refs(ZInt64Property*, size_t, uintptr_t, UObject*, const refs_callback&) {}

template <>
void find_property_refs(ZInterfaceProperty* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback) {
    callback(obj, get_property(prop, idx, base_addr));
}

template <>
void find_property_refs(ZIntProperty*, size_t, uintptr_t, UObject*, const refs_callback&) {}

template <>
void find_property_refs(ZMulticastDelegateProperty* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback) {
    auto delegate = get_property(prop, idx, base_addr);
    for (size_t i = 0; i < delegate.base->size() - 1; i++) {
        auto func = delegate.base->data[i].as_function();
        callback(obj, func->object);
        callback(obj, func->func);
    }
}

template <>
void find_property_refs(ZNameProperty*, size_t, uintptr_t, UObject*, const refs_callback&) {}

template <>
void find_property_refs(ZObjectProperty* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback) {
    callback(obj, get_property(prop, idx, base_addr));
}

template <>
void find_property_refs(ZStrProperty*, size_t, uintptr_t, UObject*, const refs_callback&) {}

template <>
void find_property_refs(ZStructProperty* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback) {
    auto value = get_property(prop, idx, base_addr);
    auto new_base = reinterpret_cast<uintptr_t>(value.base.get());

    for (auto inner_prop : value.type->properties()) {
        cast(
            inner_prop,
            [new_base, obj, &callback]<typename T>(T* prop) {
                for (size_t i = 0; i < static_cast<size_t>(prop->ArrayDim()); i++) {
                    find_property_refs(prop, i, new_base, obj, callback);
                }
            },
            // Fallback: ignore this property, assume no refs
            [](ZProperty* /*prop*/) {});
    }
}

template <>
void find_property_refs(ZTextProperty*, size_t, uintptr_t, UObject*, const refs_callback&) {}
template <>
void find_property_refs(ZUInt16Property*, size_t, uintptr_t, UObject*, const refs_callback&) {}
template <>
void find_property_refs(ZUInt32Property*, size_t, uintptr_t, UObject*, const refs_callback&) {}
template <>
void find_property_refs(ZUInt64Property*, size_t, uintptr_t, UObject*, const refs_callback&) {}

// ======== Fourth Layer Subclasses ========

template <>
void find_property_refs(ZByteAttributeProperty*,
                        size_t,
                        uintptr_t,
                        UObject*,
                        const refs_callback&) {}

template <>
void find_property_refs(ZClassProperty* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback) {
    callback(obj, get_property(prop, idx, base_addr));
}

template <>
void find_property_refs(ZComponentProperty* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback) {
    callback(obj, get_property(prop, idx, base_addr));
}

template <>
void find_property_refs(ZFloatAttributeProperty*,
                        size_t,
                        uintptr_t,
                        UObject*,
                        const refs_callback&) {}

template <>
void find_property_refs(ZGbxInlineStructProperty*,
                        size_t,
                        uintptr_t,
                        UObject*,
                        const refs_callback&) {
    // TODO OAK2
}

template <>
void find_property_refs(ZIntAttributeProperty*, size_t, uintptr_t, UObject*, const refs_callback&) {
}

template <>
void find_property_refs(ZLazyObjectProperty* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback) {
    callback(obj, get_property(prop, idx, base_addr));
}

template <>
void find_property_refs(ZSoftObjectProperty* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback) {
    callback(obj, get_property(prop, idx, base_addr));
}

template <>
void find_property_refs(ZWeakObjectProperty* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback) {
    callback(obj, get_property(prop, idx, base_addr));
}

// ======== Fifth Layer Subclasses ========

template <>
void find_property_refs(ZSoftClassProperty* prop,
                        size_t idx,
                        uintptr_t base_addr,
                        UObject* obj,
                        const refs_callback& callback) {
    callback(obj, get_property(prop, idx, base_addr));
}

// NOLINTEND(readability-named-parameter)
// =================================================================================================

void search_for_refs(UObject* from_obj, const refs_callback& callback) {
    cast<cast_options<true, true>>(
        from_obj, [&callback]<typename T>(T* obj) { find_native_refs<T>(obj, callback); });

    auto base_addr = reinterpret_cast<uintptr_t>(from_obj);
    for (auto prop : from_obj->Class()->properties()) {
        cast<cast_options<false, true>>(
            prop,
            [base_addr, from_obj, &callback]<typename T>(T* prop) {
                auto array_dim = (size_t)prop->ArrayDim();
                for (size_t i = 0; i < array_dim; i++) {
                    find_property_refs<T>(prop, i, base_addr, from_obj, callback);
                }
            },
            // Fallback: ignore this property, assume no refs
            [](ZProperty* /*prop*/) {});
    }
}

}  // namespace live_object_explorer::refs
