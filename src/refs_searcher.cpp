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
void find_native_refs(T* obj, const refs_callback& callback);

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

template <>
void find_native_refs(ZSoftClassProperty* obj, const refs_callback& callback) {
    find_native_refs<ZSoftObjectProperty>(obj, callback);
}

// =================================================================================================

// =================================================================================================

void search_for_refs(UObject* from_obj, const refs_callback& callback) {
    cast<cast_options<true, true>>(
        from_obj, [&callback]<typename T>(T* obj) { find_native_refs<T>(obj, callback); });
}

}  // namespace live_object_explorer::refs
