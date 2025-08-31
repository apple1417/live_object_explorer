#ifndef GUI_H
#define GUI_H

#include "pch.h"

namespace live_object_explorer::gui {

/**
 * @brief Opens the gui, if it isn't already.
 */
void show(void);

/**
 * @brief Perform a new search.
 *
 * @param query The search query.
 */
void search(std::string_view query);

/**
 * @brief Checks if any part of the gui is open.
 *
 * @return True if it's open.
 */
bool is_open(void);

/**
 * @brief Renders the gui.
 */
void render(void);

/**
 * @brief Opens a new object window.
 *
 * @param obj The object shown in the window.
 * @param parent_window The id of the parent window to initially dock this to. Defaults to the first
 *                      open object window when not given.
 */
void open_object_window(unrealsdk::unreal::UObject* obj);
void open_object_window(unrealsdk::unreal::UObject* obj, const std::string& parent_window_id);

}  // namespace live_object_explorer::gui

#endif /* GUI_H */
