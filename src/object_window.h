#ifndef OBJECT_WINDOW_H
#define OBJECT_WINDOW_H

#include "pch.h"

namespace live_object_explorer {

class ObjectWindow {
   private:
    unrealsdk::unreal::WeakPointer ptr;

    std::string id;
    std::string name;

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
};

}  // namespace live_object_explorer

#endif /* OBJECT_WINDOW_H */
