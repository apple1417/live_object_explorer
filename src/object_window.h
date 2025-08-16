#ifndef OBJECT_WINDOW_H
#define OBJECT_WINDOW_H

#include "pch.h"

namespace live_object_explorer {

class AbstractComponent;

struct ObjectWindowSettings {
    bool editable = false;
    bool hex = false;

    ImGuiTextFilter filter;
    bool filter_active_last_time = false;
};

class ObjectWindow {
   public:
    /**
     * @brief Parses the given object into a format ready to be drawn to screen.
     *
     * @param obj The object to parse.
     */
    ObjectWindow(unrealsdk::unreal::UObject* obj);

    ~ObjectWindow() = default;

    ObjectWindow(ObjectWindow&&) = delete;
    ObjectWindow(const ObjectWindow&) = delete;
    ObjectWindow& operator=(const ObjectWindow&) = delete;
    ObjectWindow& operator=(ObjectWindow&&) = delete;

    /**
     * @brief Gets the id to use for this window.
     */
    [[nodiscard]] const std::string& get_id(void) const;

    /**
     * @brief Draws this object into an existing window.
     * @note Does not call begin/end.
     */
    void draw(void);

   private:
    unrealsdk::unreal::WeakPointer ptr;

    std::string id;
    std::string name;

    struct ClassSection {
        std::string header;
        std::vector<std::unique_ptr<AbstractComponent>> components;
        bool was_force_closed;
    };

    std::vector<ClassSection> prop_sections;
    std::vector<ClassSection> field_sections;

    ObjectWindowSettings settings = {};
};

}  // namespace live_object_explorer

#endif /* OBJECT_WINDOW_H */
