#ifndef COMPONENT_PICKER_H
#define COMPONENT_PICKER_H

#include "pch.h"

namespace live_object_explorer {

class AbstractComponent;

/**
 * @brief Adds new components for the given field to the given list.
 *
 * @param components The list of components to add to.
 * @param field The field to add.
 * @param base_addr The base address of the object/struct this field is on.
 */
void insert_component(std::vector<std::unique_ptr<AbstractComponent>>& components,
                      unrealsdk::unreal::UField* field,
                      uintptr_t base_addr);

}  // namespace live_object_explorer

#endif /* COMPONENT_PICKER_H */
