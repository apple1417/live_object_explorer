#include "pch.h"
#include "object_window.h"
#include "components/abstract.h"
#include "components/integral_component.h"
#include "components/object_component.h"
#include "components/unknown_component.h"

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
                                        unrealsdk::utils::narrow(obj->get_path_name()))) {
    for (UStruct* cls = obj->Class(); cls != nullptr; cls = cls->SuperField()) {
        decltype(ClassSection::components) components{};
        for (auto field = cls->Children(); field != nullptr; field = field->Next()) {
            if (!field->Class()->inherits(find_class<UProperty>())) {
                // TODO own list?
                continue;
            }

            // TODO: cast, component per type
            components.emplace_back(std::make_unique<UnknownComponent>(
                (std::string)field->Name(), (std::string)field->Class()->Name()));
        }

        this->sections.emplace_back(
            std::format("{}{}##section_{}", cls->Name(), components.empty() ? " (empty)" : "",
                        this->sections.size()),
            std::move(components));
    }
    this->append_native_section(obj);
}

const std::string& ObjectWindow::get_id() const {
    return this->id;
}

void ObjectWindow::draw() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Settings")) {
            ImGui::MenuItem("Enable Editing", nullptr, &this->settings.editable);
            ImGui::MenuItem("Hex Numbers", nullptr, &this->settings.hex);
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::Text("Dump for %s", this->name.c_str());
    if (!ptr) {
        ImGui::TextDisabled("Object has been garbage collected");

        // Might as well free up some memory early
        this->sections.clear();
        return;
    }

    for (auto& section : this->sections) {
        if (ImGui::TreeNodeEx(section.header.c_str(),
                              section.components.empty() ? ImGuiTreeNodeFlags_Leaf : 0)) {
            for (auto& component : section.components) {
                component->draw(this->settings);
            }
            ImGui::TreePop();
        }
    }
}

void ObjectWindow::append_native_section(UObject* obj) {
    auto section_name = std::format("Native##section_{}", this->sections.size());
    this->sections.emplace_back(section_name, decltype(ClassSection::components){});
    auto& components = this->sections.back().components;

    components.emplace_back(
        std::make_unique<IntegralComponent<std::remove_reference_t<decltype(obj->ObjectFlags())>>>(
            "ObjectFlags", &obj->ObjectFlags()));
    components.emplace_back(
        std::make_unique<
            IntegralComponent<std::remove_reference_t<decltype(obj->InternalIndex())>>>(
            "InternalIndex", &obj->InternalIndex()));

    // TODO: proper class component
    components.emplace_back(
        std::make_unique<ObjectComponent>("Class", reinterpret_cast<UObject**>(&obj->Class())));

    // TODO: Name
    components.emplace_back(std::make_unique<ObjectComponent>("Outer", &obj->Outer()));

    // TODO: other classes
    // TODO: gaps?
}

}  // namespace live_object_explorer
