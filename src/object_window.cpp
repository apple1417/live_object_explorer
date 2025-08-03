#include "pch.h"
#include "object_window.h"
#include "component_picker.h"
#include "components/abstract.h"
#include "components/object_component.h"
#include "components/scalar_component.h"
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
        this->sections.emplace_back(
            std::format("{}##section_{}", cls->Name(), this->sections.size()),
            decltype(ClassSection::components){});

        auto& components = this->sections.back().components;
        for (auto field = cls->Children(); field != nullptr; field = field->Next()) {
            insert_component(components, field, reinterpret_cast<uintptr_t>(obj));
        }
    }
    this->append_native_section(obj);
}

const std::string& ObjectWindow::get_id() const {
    return this->id;
}

void ObjectWindow::draw() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Settings")) {
            // TODO
            // ImGui::MenuItem("Enable Editing", nullptr, &this->settings.editable);
            ImGui::MenuItem("Hex Integers", nullptr, &this->settings.hex);
            ImGui::MenuItem("Show UFields", nullptr, &this->settings.include_fields);
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

    auto content_region = ImGui::GetContentRegionAvail();
    // NOLINTBEGIN(readability-magic-numbers)
    float width =
        -(content_region.x - std::min(ImGui::GetFontSize() * 20.0F, content_region.x * 0.5F));
    // NOLINTEND(readability-magic-numbers)

    this->filter.Draw("Filter", width);
    auto filter_active = this->filter.IsActive();

    for (auto& section : this->sections) {
        // When you start using the filter, force all nodes open
        // When you stop, force all nodes closed
        if (filter_active != this->filter_active_last_time) {
            ImGui::SetNextItemOpen(filter_active);
        }

        if (ImGui::TreeNode(section.header.c_str())) {
            ImGui::PushItemWidth(width);

            for (auto& component : section.components) {
                if (component->passes_filter(this->filter)) {
                    component->draw(this->settings);
                }
            }

            ImGui::PopItemWidth();
            ImGui::TreePop();
        }
    }

    this->filter_active_last_time = filter_active;
}

void ObjectWindow::append_native_section(UObject* obj) {
    auto section_name = std::format("Native##section_{}", this->sections.size());
    this->sections.emplace_back(section_name, decltype(ClassSection::components){});
    auto& components = this->sections.back().components;

    components.emplace_back(
        // Read off the real type of object flags, since it changes
        std::make_unique<ScalarComponent<std::remove_reference_t<decltype(obj->ObjectFlags())>>>(
            "ObjectFlags", &obj->ObjectFlags()));
    components.emplace_back(std::make_unique<IntComponent>("InternalIndex", &obj->InternalIndex()));

    // TODO: proper class component
    components.emplace_back(std::make_unique<ObjectComponent>(
        "Class", reinterpret_cast<UObject**>(&obj->Class()), find_class<UClass>()));

    // TODO: Name
    components.emplace_back(
        std::make_unique<ObjectComponent>("Outer", &obj->Outer(), find_class<UObject>()));

    // TODO: other classes
    // TODO: gaps?
}

}  // namespace live_object_explorer
