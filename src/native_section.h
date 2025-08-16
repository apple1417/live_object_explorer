#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

/**
 * @brief Inserts all components sourced from native object fields to the given list.
 *
 * @param components The list of components to add to.
 * @param obj The object to gather native components of.
 */
void insert_all_native_components(std::vector<std::unique_ptr<AbstractComponent>>& components,
                                  unrealsdk::unreal::UObject* obj);

}  // namespace live_object_explorer
