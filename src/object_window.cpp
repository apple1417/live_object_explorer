#include "pch.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

namespace {

// Used to ensure we have unique ids
std::atomic<size_t> object_window_counter = 0;

}  // namespace

ObjectWindow::ObjectWindow(UObject* obj)
    : ptr(obj),
      id(std::format("{}##object_{}",
                     obj == nullptr ? "Unknown Object" : (std::string)obj->Name(),
                     object_window_counter++)),
      name(obj == nullptr ? "Unknown Object"
                          : std::format("{}'{}'",
                                        obj->Class()->Name(),
                                        unrealsdk::utils::narrow(obj->get_path_name()))) {}

const std::string& ObjectWindow::get_id() const {
    return this->id;
}

void ObjectWindow::draw() {
    ImGui::Text("%s", this->name.c_str());
}

}  // namespace live_object_explorer
