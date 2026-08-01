#ifndef REFS_SEARCHER_H
#define REFS_SEARCHER_H

#include "pch.h"

namespace live_object_explorer::refs {

using refs_callback =
    std::function<void(unrealsdk::unreal::UObject* from_obj, unrealsdk::unreal::UObject* to_obj)>;

/**
 * @brief Searches for spots where the given object references others.
 *
 * @param from_obj The object to search from.
 * @param callback A callback to call with any discovered refs.
 */
void search_for_refs(unrealsdk::unreal::UObject* from_obj, const refs_callback& callback);

}  // namespace live_object_explorer::refs

#endif /* REFS_SEARCHER_H */
