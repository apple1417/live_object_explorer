#include "pch.h"
#include "components/struct_field_component.h"
#include "gui.h"
#include "object_window.h"

using namespace unrealsdk::unreal;

namespace live_object_explorer {

namespace {

using property_flags_type = unrealsdk::unreal::UProperty::property_flags_type;

/**
 * @brief Appends function flags to the given property name.
 *
 * @param name The property name to append to.
 * @param flags The flags to check through.
 */
void append_function_flags(std::string& name, property_flags_type flags) {
    if ((flags & UProperty::PROP_FLAG_PARAM) == 0) {
        return;
    }

    const bool is_return = (flags & UProperty::PROP_FLAG_RETURN) != 0;
    const bool is_out = (flags & UProperty::PROP_FLAG_OUT) != 0;
#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
    const bool is_optional = (flags & UProperty::PROP_FLAG_OPTIONAL) != 0;
#else
    const bool is_optional = false;
#endif

    if (is_return || is_out || is_optional) {
        // Hacky magic just to not bother with a string stream
        // NOLINTNEXTLINE(readability-magic-numbers)
        const size_t length = 1 + (is_return ? 8 : 0) + (is_out ? 5 : 0) + (is_optional ? 10 : 0);
        name.reserve(name.size() + length);

        name += " (";
        if (is_return) {
            name += "return, ";
        }
        if (is_out) {
            name += "out, ";
        }
        if (is_optional) {
            name += "optional, ";
        }

        // Replace trailing comma + space with closing bracket
        name.pop_back();
        name.back() = ')';
    }
}

}  // namespace

StructFieldComponent::StructFieldComponent(std::string&& name, UStruct* ustruct)
    : ObjectFieldComponent(std::move(name), ustruct),
      is_function(ustruct->is_instance(find_class<UFunction>())) {
    for (auto prop : ustruct->properties()) {
        this->properties.emplace_back();
        auto& data = this->properties.back();

        data.name = prop->Name();
        if (this->is_function) {
            append_function_flags(data.name, prop->PropertyFlags());
        }

        // Future improvement: parse the actual property into a friendly name, e.g.
        // - bool
        // - array<SomeStructType>
        // - ESomeEnum
        // - WillowPlayerController
        data.type = prop->Class()->Name();
        data.link = prop;
    }
}

void StructFieldComponent::draw(const ObjectWindowSettings& /*settings*/,
                                ForceExpandTree expand_children,
                                bool /*show_all_children*/) {
    if (expand_children != ForceExpandTree::NONE) {
        ImGui::SetNextItemOpen(expand_children == ForceExpandTree::OPEN);
    }

    if (ImGui::TreeNode(this->name.c_str(), "%s %s", this->is_function ? "Function" : "Struct",
                        this->hashless_name.c_str())) {
        if (this->ptr) {
            if (ImGui::TextLink(this->cached_obj_name.c_str())) {
                gui::open_object_window(*this->ptr, this->name);
            }
        } else {
            ImGui::TextDisabled("%s", this->cached_obj_name.c_str());
        }

        for (const auto& prop_info : this->properties) {
            ImGui::BulletText("%s:", prop_info.name.c_str());
            ImGui::SameLine();
            if (prop_info.link) {
                if (ImGui::TextLink(prop_info.type.c_str())) {
                    gui::open_object_window(*prop_info.link, this->name);
                }
            } else {
                ImGui::Text("%s", prop_info.type.c_str());
            }
        }

        ImGui::TreePop();
    }
}

}  // namespace live_object_explorer
