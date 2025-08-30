#include "pch.h"
#include "components/enum_field_component.h"
#include "object_link.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

EnumFieldComponent::EnumFieldComponent(std::string&& name, UEnum* uenum)
    : ObjectFieldComponent(std::move(name), uenum) {
    std::ranges::copy(uenum->get_names() | std::views::transform([](auto pair) {
                          return EnumNameInfo{
                              std::format("{} ({})", pair.first, pair.second),
                              std::format("{} ({:X})", pair.first, pair.second),
                          };
                      }),
                      std::back_inserter(this->name_info));
}

void EnumFieldComponent::draw(const ObjectWindowSettings& settings,
                              ForceExpandTree expand_children,
                              bool /*show_all_children*/) {
    if (expand_children != ForceExpandTree::NONE) {
        ImGui::SetNextItemOpen(expand_children == ForceExpandTree::OPEN);
    }

    if (ImGui::TreeNodeEx(this->name.c_str(), ImGuiTreeNodeFlags_DrawLinesFull, "Enum %s",
                          this->name.c_str())) {
        ImGui::TableNextColumn();

        object_link(this->cached_obj_name, *this->ptr);

        for (const auto& info : this->name_info) {
            ImGui::PushID(&info);

            ImGui::BulletText("%s", settings.hex ? info.hex.c_str() : info.decimal.c_str());

            ImGui::PopID();
        }

        ImGui::TreePop();
    }
}

}  // namespace live_object_explorer
