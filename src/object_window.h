#ifndef OBJECT_WINDOW_H
#define OBJECT_WINDOW_H

#include "pch.h"

namespace live_object_explorer {

class AbstractComponent;

struct ObjectWindowSettings {
    bool editable = false;
    bool hex = false;
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
    };

    std::vector<ClassSection> sections;

    ObjectWindowSettings settings = {};

    /**
     * @brief Adds the section for native properties to the end of the list.
     *
     * @param obj The object being initalized.
     */
    void append_native_section(unrealsdk::unreal::UObject* obj);
};

}  // namespace live_object_explorer

#endif /* OBJECT_WINDOW_H */
