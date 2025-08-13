#ifndef COMPONENT_PICKER_H
#define COMPONENT_PICKER_H

#include "pch.h"

namespace live_object_explorer {

class AbstractComponent;

/**
 * @brief Adds new components for fields extracted from an object/struct to the given list.
 *
 * @param prop_components The list of components to add properties to.
 * @param field_components The list of components to add non-property fields to.
 * @param obj The object to add.
 * @param base_addr The base address of the object/struct this field is on.
 */
void insert_component(std::vector<std::unique_ptr<AbstractComponent>>& prop_components,
                      std::vector<std::unique_ptr<AbstractComponent>>& field_components,
                      unrealsdk::unreal::UObject* obj,
                      uintptr_t base_addr);

/**
 * @brief Adds new components for fields extracted from an array to the given list.
 *
 * @param prop_components The list of components to add to.
 * @param arr The array to look under.
 * @param inner_prop The array's inner property.
 * @param idx The index in the array to get components for.
 */
void insert_component_array(std::vector<std::unique_ptr<AbstractComponent>>& prop_components,
                            unrealsdk::unreal::TArray<void>* arr,
                            unrealsdk::unreal::UProperty* inner_prop,
                            size_t idx);

}  // namespace live_object_explorer

#endif /* COMPONENT_PICKER_H */
