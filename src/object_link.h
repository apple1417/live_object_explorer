#ifndef OBJECT_LINK_H
#define OBJECT_LINK_H

#include "pch.h"

namespace live_object_explorer {

const constexpr std::string_view NULL_OBJECT_NAME = "None";

/**
 * @brief Formats an object reference into a name.
 *
 * @param obj The object to get the name of. May be null.
 * @param hash A hash to ensure the object name is unique.
 */
std::string format_object_name(unrealsdk::unreal::UObject* obj, std::string_view hash);

/**
 * @brief Draws a link to another object.
 *
 * @param text The link's text.
 * @param obj The object to link to. May be null.
 * @param obj_getter A function to lazily get the object. Using this assumes it's non-null.
 * @param parent_window The id of the parent window to dock, if this opens a new one.
 */
void object_link(const std::string& text,
                 unrealsdk::unreal::UObject* obj,
                 const std::string& parent_window_id);
void object_link(const std::string& text,
                 const std::function<unrealsdk::unreal::UObject*(void)>& obj_getter,
                 const std::string& parent_window_id);

// Class to automatically keep track of an object name
struct CachedObjLink {
   private:
    uintptr_t addr;
    std::string name;
    std::string_view hash;

   public:
    /**
     * @brief Constructs a new cached object link.
     *
     * @param hash A hash to ensure the object name is unique.
     */
    CachedObjLink(std::string_view hash);

    /**
     * @brief Updates the cache and draws an object link.
     *
     * @param obj The current object.
     * @param parent_window The id of the parent window to dock, if this opens a new one.
     */
    void draw(unrealsdk::unreal::UObject* obj, const std::string& parent_window_id);

    /**
     * @brief Checks if this object passes the given text filter.
     *
     * @param filter The filter to check.
     * @return True if it passes.
     */
    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter);
};

}  // namespace live_object_explorer

#endif /* OBJECT_LINK_H */
