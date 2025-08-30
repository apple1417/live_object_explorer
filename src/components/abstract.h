#ifndef COMPONENTS_ABSTRACT_H
#define COMPONENTS_ABSTRACT_H

#include "pch.h"

namespace live_object_explorer {

struct ObjectWindowSettings;

enum class ForceExpandTree : uint8_t {
    NONE,
    OPEN,
    CLOSE,
};

class AbstractComponent {
   protected:
    std::string name;

   public:
    /**
     * @brief Constructs a new component.
     *
     * @param name The component's name. Should not include hashes.
     */
    AbstractComponent(std::string&& name);

    virtual ~AbstractComponent() = default;

    /**
     * @brief Draws this component.
     *
     * @param settings Settings from the parent window.
     * @param expand_children If to force expand any nested tree nodes this component draws.
     * @param show_all_children When true, show all nested children, don't filter them.
     */
    virtual void draw(const ObjectWindowSettings& settings,
                      ForceExpandTree expand_children,
                      bool show_all_children) = 0;

    /**
     * @brief Checks if this component passes the given text filter.
     *
     * @param filter The filter to check.
     * @return True if it passes.
     */
    [[nodiscard]] virtual bool passes_filter(const ImGuiTextFilter& filter);
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_ABSTRACT_H */
