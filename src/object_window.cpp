#include "pch.h"
#include "object_window.h"
#include "component_picker.h"
#include "components/abstract.h"
#include "components/object_component.h"
#include "components/scalar_component.h"

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
        this->prop_sections.emplace_back(
            std::format("{}##prop_section_{}", cls->Name(), this->prop_sections.size()),
            decltype(ClassSection::components){});
        this->field_sections.emplace_back(
            std::format("{}##field_section_{}", cls->Name(), this->prop_sections.size()),
            decltype(ClassSection::components){});

        auto& prop_components = this->prop_sections.back().components;
        auto& field_components = this->field_sections.back().components;
        for (auto field = cls->Children(); field != nullptr; field = field->Next()) {
            insert_component(prop_components, field_components, field,
                             reinterpret_cast<uintptr_t>(obj));
        }
    }
    this->append_native_section(obj);
}

const std::string& ObjectWindow::get_id() const {
    return this->id;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void ObjectWindow::draw() {
    if (ImGui::BeginMenuBar()) {
        // TODO
        ImGui::MenuItem("Enable Editing", nullptr, &this->settings.editable, false);
        ImGui::MenuItem("Hex Integers", nullptr, &this->settings.hex);

        ImGui::EndMenuBar();
    }

    ImGui::Text("Dump for %s", this->name.c_str());
    if (!ptr) {
        ImGui::TextDisabled("Object has been garbage collected");

        // Might as well free up some memory early
        this->prop_sections.clear();
        this->field_sections.clear();
        return;
    }

    auto content_region = ImGui::GetContentRegionAvail();
    // NOLINTBEGIN(readability-magic-numbers)
    float width =
        -(content_region.x - std::min(ImGui::GetFontSize() * 20.0F, content_region.x * 0.5F));
    // NOLINTEND(readability-magic-numbers)

    this->settings.filter.Draw("Filter", width);
    auto filter_active = this->settings.filter.IsActive();

    auto draw_sections = [this, filter_active, width](std::vector<ClassSection>& section_list) {
        for (auto& section : section_list) {
            // When you start using the filter, force all nodes open
            // When you stop, force all nodes closed
            ForceExpandTree expand_children = ForceExpandTree::NONE;
            if (filter_active != this->settings.filter_active_last_time) {
                ImGui::SetNextItemOpen(filter_active);
                expand_children = filter_active ? ForceExpandTree::OPEN : ForceExpandTree::CLOSE;

                // Now unfortunately, when we force close, TreeNode returns false, so we never draw
                // any of the components, and never tell them to close. Instead, we'll store that we
                // were force closed (clearing if we get force opened), so we can apply it when we
                // next get opened
                section.was_force_closed = !filter_active;
            }

            if (ImGui::TreeNode(section.header.c_str())) {
                ImGui::PushItemWidth(width);

                for (auto& component : section.components) {
                    if (component->passes_filter(this->settings.filter)) {
                        component->draw(
                            this->settings,
                            section.was_force_closed ? ForceExpandTree::CLOSE : expand_children,
                            false);
                    }
                }

                section.was_force_closed = false;

                ImGui::PopItemWidth();
                ImGui::TreePop();
            }
        }
    };

    ImGui::SeparatorText("Properties");
    draw_sections(this->prop_sections);
    ImGui::SeparatorText("Class Fields");
    draw_sections(this->field_sections);

    this->settings.filter_active_last_time = filter_active;
}

void ObjectWindow::append_native_section(UObject* obj) {
    auto section_name = std::format("Native##section_{}", this->prop_sections.size());
    this->prop_sections.emplace_back(section_name, decltype(ClassSection::components){});
    auto& components = this->prop_sections.back().components;

    components.emplace_back(
        // Read off the real type of object flags, since it changes
        std::make_unique<ScalarComponent<std::remove_reference_t<decltype(obj->ObjectFlags())>>>(
            "ObjectFlags", &obj->ObjectFlags()));
    components.emplace_back(
        std::make_unique<Int32Component>("InternalIndex", &obj->InternalIndex()));

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
