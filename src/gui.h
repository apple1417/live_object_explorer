#ifndef GUI_H
#define GUI_H

namespace live_object_explorer::gui {

/**
 * @brief Checks if the gui is open.
 *
 * @return True if the gui is open.
 */
bool is_open(void);

/**
 * @brief Renders the gui.
 */
void render(void);

}  // namespace live_object_explorer::gui

#endif /* GUI_H */
