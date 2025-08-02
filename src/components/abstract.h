#ifndef COMPONENTS_ABSTRACT_H
#define COMPONENTS_ABSTRACT_H

#include "pch.h"

namespace live_object_explorer {

struct ObjectWindowSettings;

class AbstractComponent {
   public:
    virtual ~AbstractComponent() = default;

    /**
     * @brief Draws this component.
     *
     * @param settings Settings from the parent window.
     */
    virtual void draw(const ObjectWindowSettings& settings) = 0;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_ABSTRACT_H */
