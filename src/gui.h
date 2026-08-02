#ifndef GUI_H
#define GUI_H

#include "pch.h"

namespace live_object_explorer::gui {

struct SearchResult {
    // Ignores the pointer, looks the object up by name when selected
    static constexpr auto NOT_LIVE = 1 << 0;
    // If not live, the object was selected at some point, and we couldn't find it
    static constexpr auto LOOKUP_FAILED = 1 << 1;

    std::string name;                              // The object path name
    unrealsdk::unreal::WeakPointer ptr = nullptr;  // A weak pointer to the object
    uint8_t flags = 0;                             // Search result flags
};

/**
 * @brief Opens the gui, if it isn't already.
 */
void show(void);

/**
 * @brief Perform a new live search, in response to the console command.
 *
 * @param query The search query.
 */
void search_cmd(std::string_view query);

/**
 * @brief Perform a new "refs to" search.
 *
 * @param query The search query.
 */
void search_refs_to(std::string_view query);

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
 * @param var The object shown in the window.
 * @param parent_window The id of the parent window to initially dock this to. Defaults to the first
 *                      open object window when not given.
 */
void open_object_window(const unrealsdk::unreal::FFieldVariant& var);
void open_object_window(const unrealsdk::unreal::FFieldVariant& var,
                        const std::string& parent_window_id);

}  // namespace live_object_explorer::gui

#endif /* GUI_H */
