#include "pch.h"
#include "gui.h"

namespace live_object_explorer::gui {

bool is_open(void) {
    return true;
}

void render(void) {
    ImGui::ShowDemoWindow();
}

}  // namespace live_object_explorer::gui
