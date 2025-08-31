#include "pch.h"
#include "object_window.h"
#include "component_picker.h"
#include "components/abstract.h"
#include "imgui.h"
#include "native_section.h"
#include "object_link.h"

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
      name(obj == nullptr ? "Unknown Object" : format_object_name(obj)) {
    for (UStruct* cls = obj->Class(); cls != nullptr; cls = cls->SuperField()) {
        this->prop_sections.emplace_back((std::string)cls->Name(),
                                         decltype(ClassSection::components){});
        this->field_sections.emplace_back((std::string)cls->Name(),
                                          decltype(ClassSection::components){});

        auto& prop_components = this->prop_sections.back().components;
        auto& field_components = this->field_sections.back().components;
        for (auto field = cls->Children(); field != nullptr; field = field->Next()) {
            insert_component(prop_components, field_components, field,
                             reinterpret_cast<uintptr_t>(obj));
        }
    }

    this->prop_sections.emplace_back("Native", decltype(ClassSection::components){});
    insert_all_native_components(this->prop_sections.back().components, obj);
}

const std::string& ObjectWindow::get_id() const {
    return this->id;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void ObjectWindow::draw() {
    if (ImGui::BeginMenuBar()) {
        ImGui::MenuItem("Enable Editing", nullptr, &this->settings.editable);
        ImGui::MenuItem("Hex Integers", nullptr, &this->settings.hex);

        ImGui::EndMenuBar();
    }

    ImGui::Text("Dump for");
    ImGui::SameLine();
    object_link(this->name, *this->ptr);

    if (!ptr) {
        ImGui::TextDisabled("Object has been garbage collected");

        // Might as well free up some memory early
        this->prop_sections.clear();
        this->field_sections.clear();
        return;
    }

    this->settings.filter.Draw(
        "Filter", -(ImGui::CalcTextSize("Filter").x + (2 * ImGui::GetStyle().ItemSpacing.x)));
    auto filter_active = this->settings.filter.IsActive();

    auto draw_sections = [this, filter_active](std::vector<ClassSection>& section_list) {
        for (auto& section : section_list) {
            ImGui::PushID(&section);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

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

            if (ImGui::TreeNodeEx(section.header.c_str(), ImGuiTreeNodeFlags_DrawLinesFull)) {
                for (auto& component : section.components) {
                    if (component->passes_filter(this->settings.filter)) {
                        ImGui::PushID(&component);
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();

                        component->draw(
                            this->settings,
                            section.was_force_closed ? ForceExpandTree::CLOSE : expand_children,
                            false);

                        ImGui::PopID();
                    }
                }

                section.was_force_closed = false;
                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    };

    ImGui::SeparatorText("Properties");
    if (ImGui::BeginTable("props", 2,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV
                              | ImGuiTableFlags_NoSavedSettings)) {
        draw_sections(this->prop_sections);
        ImGui::EndTable();
    }

    ImGui::SeparatorText("Class Fields");
    if (ImGui::BeginTable("fields", 2,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV
                              | ImGuiTableFlags_NoSavedSettings)) {
        draw_sections(this->field_sections);
        ImGui::EndTable();
    }

    this->settings.filter_active_last_time = filter_active;
}

}  // namespace live_object_explorer
