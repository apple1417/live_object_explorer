#include "pch.h"
#include "gui.h"

namespace live_object_explorer::gui {

namespace {

bool showing = false;

}

void show(void) {
    showing = true;
}

void render(void) {
    if (!showing) {
        return;
    }

    ImGui::ShowDemoWindow(&showing);
}

}  // namespace live_object_explorer::gui
