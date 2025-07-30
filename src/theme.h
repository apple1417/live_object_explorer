#ifndef THEME_H
#define THEME_H

#include "pch.h"
#include "injected_imgui/auto.h"

namespace live_object_explorer::theme {

/**
 * @brief Initalizes the themes module.
 *
 * @param api Which graphics api was used to hook imgui.
 */
void init(injected_imgui::auto_detect::Api api);

/**
 * @brief Applies the requested theme.
 */
void apply(void);

}  // namespace live_object_explorer::theme

#endif /* THEME_H */
