#ifndef OBJECT_LINK_H
#define OBJECT_LINK_H

#include "pch.h"

namespace live_object_explorer {

const std::string NULL_OBJECT_NAME = "None";

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

    std::string link_name;
    std::string_view hash;
    size_t length_before_hash;

    std::string editable_name;

    std::string failed_to_set_msg;

    /**
     * @brief Updates the cached object info.
     *
     * @param obj The current object.
     */
    void update_obj(unrealsdk::unreal::UObject* obj);

   public:
    /**
     * @brief Constructs a new cached object link.
     *
     * @param hash A hash added to the name used in links to ensure it's unique.
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
     * @brief Updates the cache and draws an object input.
     *
     * @param obj The current object.
     * @param label The input box's label.
     * @param setter Called with a candidate object (possibly null) to try set.
     */
    void draw_editable(unrealsdk::unreal::UObject* obj,
                       const std::string& label,
                       const std::function<void(unrealsdk::unreal::UObject*)>& setter);

    /**
     * @brief Shows a popup window explaining why a setter failed.
     * @note The popup is only drawn by draw_editable.
     *
     * @param msg The reason for failure.
     */
    void fail_to_set(std::string&& msg);

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
