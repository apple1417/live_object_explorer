#ifndef OBJECT_LINK_H
#define OBJECT_LINK_H

#include "pch.h"

namespace live_object_explorer {

const std::string NULL_OBJECT_NAME = "None";

/**
 * @brief Formats an object reference into a name.
 *
 * @param obj The object to get the name of. May be null.
 */
std::string format_object_name(unrealsdk::unreal::UObject* obj);

/**
 * @brief Draws a link to another object.
 *
 * @param text The link's text.
 * @param obj The object to link to. May be null.
 * @param obj_getter A function to lazily get the object. Using this assumes it's non-null.
 */
void object_link(const std::string& text, unrealsdk::unreal::UObject* obj);
void object_link(const std::string& text,
                 const std::function<unrealsdk::unreal::UObject*(void)>& obj_getter);

struct CachedObjLink {
   private:
    uintptr_t addr = 0;

    std::string name = NULL_OBJECT_NAME;
    std::string editable_name;

    std::string failed_to_set_msg;

    bool pending_edit = false;

    /**
     * @brief Updates the cached object info.
     *
     * @param obj The current object.
     */
    void update_obj(unrealsdk::unreal::UObject* obj);

   public:
    /**
     * @brief Constructs a new cached object link.
     */
    CachedObjLink(void) = default;

    /**
     * @brief Updates the cache and draws an object link.
     *
     * @param obj The current object.
     */
    void draw(unrealsdk::unreal::UObject* obj);

    /**
     * @brief Updates the cache and draws an object input.
     *
     * @param obj The current object.
     * @param setter Called with a candidate object (possibly null) to try set.
     */
    void draw_editable(unrealsdk::unreal::UObject* obj,
                       const std::function<void(unrealsdk::unreal::UObject*)>& setter);

    /**
     * @brief Shows a popup window explaining why a setter failed.
     * @note May be called outside of the setter.
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
