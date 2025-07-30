#include "pch.h"
#include "gui.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace live_object_explorer::gui {

namespace {

bool showing = false;

std::atomic<size_t> obj_counter = 0;
class CachedObject {
   public:
    CachedObject(std::string_view name)
        : name(name), id(std::format("{}##obj_{}", name, obj_counter++)) {}

    std::string name;
    std::string id;

    void draw(void) const { ImGui::Text("contents of %s", name.c_str()); }
};

std::vector<std::unique_ptr<CachedObject>> objects{};

}  // namespace

void show(void) {
    showing = true;
}

void render(void) {
    if (!showing && objects.empty()) {
        return;
    }

    ImGui::ShowDemoWindow();

    if (ImGui::Begin("Live Object Explorer", &showing)) {
        // NOLINTNEXTLINE(readability-magic-numbers)
        static char search_query[4096] = "some.obj.path";
        ImGui::InputText("##searchbar", &search_query[0], IM_ARRAYSIZE(search_query));
        ImGui::SameLine();
        ImGui::Button("Dump");
        ImGui::SameLine();
        ImGui::Button("Getall");

        static std::vector<std::string> results{{"obj1", "obj2", "obj3"}};
        static size_t selected_result_idx = 0;

        if (ImGui::BeginListBox("##results listbox", ImVec2(-FLT_MIN, -FLT_MIN))) {
            for (size_t i = 0; i < results.size(); i++) {
                bool is_selected = selected_result_idx == i;
                if (ImGui::Selectable(results[i].c_str(), is_selected)) {
                    selected_result_idx = i;
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    objects.emplace_back(std::make_unique<CachedObject>(results[i]));

                    auto node_id = ImGui::GetID(objects[0]->id.c_str());
                    if (ImGui::DockBuilderGetNode(node_id) == nullptr) {
                        ImGui::DockBuilderAddNode(node_id, 0);
                        ImGui::DockBuilderSetNodePos(node_id, ImVec2(100, 100));
                        ImGui::DockBuilderSetNodeSize(node_id, ImVec2(100, 100));
                    }
                    ImGui::DockBuilderDockWindow(objects.back()->id.c_str(), node_id);
                    ImGui::DockBuilderFinish(node_id);
                }
            }
            ImGui::EndListBox();
        }
    }

    ImGui::End();

    auto it = objects.begin();
    while (it != objects.end()) {
        bool open = true;
        if (ImGui::Begin((*it)->id.c_str(), &open, ImGuiWindowFlags_NoSavedSettings)) {
            (*it)->draw();
        }
        ImGui::End();

        if (it == objects.begin()) {
            auto node_id = ImGui::GetID(objects[0]->id.c_str());
            ImGui::DockSpace(node_id);
        }

        if (!open) {
            it = objects.erase(it);
        } else {
            it++;
        }
    }
}

}  // namespace live_object_explorer::gui
