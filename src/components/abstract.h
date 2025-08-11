#ifndef COMPONENTS_ABSTRACT_H
#define COMPONENTS_ABSTRACT_H

#include "pch.h"

namespace live_object_explorer {

struct ObjectWindowSettings;

class AbstractComponent {
   protected:
    std::string name;
    size_t length_before_hash;

   public:
    /**
     * @brief Constructs a new component.
     *
     * @param name The component's name. May include hashes.
     */
    AbstractComponent(std::string&& name);

    virtual ~AbstractComponent() = default;

    /**
     * @brief Draws this component.
     *
     * @param settings Settings from the parent window.
     */
    virtual void draw(const ObjectWindowSettings& settings) = 0;

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
