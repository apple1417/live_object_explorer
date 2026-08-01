#ifndef REFS_H
#define REFS_H

#include "pch.h"

namespace live_object_explorer::refs {

// Variable controlling how many threads we use while taking a snapshot.
extern uint32_t num_threads;

/**
 * @brief Initializes the references modules.
 */
void init(void);

/**
 * @brief Checks if we have a refs snapshot, and are able to search.
 *
 * @return True if we have a snapshot.
 */
bool has_snapshot(void);

/**
 * @brief Takes a new refs snapshot.
 */
void take_snapshot(void);

/**
 * @brief Imports a refs db from disk.
 */
void import_db(void);

/**
 * @brief Exports the refs db to disk.
 */
void export_db(void);

}  // namespace live_object_explorer::refs

#endif /* REFS_H */
