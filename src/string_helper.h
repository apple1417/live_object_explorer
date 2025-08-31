#ifndef STRING_HELPER_H
#define STRING_HELPER_H

#include "pch.h"

namespace live_object_explorer {

/**
 * @brief Callback for InputText functions, to resize a std:string.
 * @note Instance of ImGuiInputTextCallback
 * @note The user data must be set to a pointer to the std::string.
 */
int string_resize_callback(ImGuiInputTextCallbackData* data);

static_assert(std::is_convertible_v<decltype(string_resize_callback), ImGuiInputTextCallback>);

}  // namespace live_object_explorer

#endif /* STRING_HELPER_H */
