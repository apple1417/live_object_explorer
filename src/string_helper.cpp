#include "pch.h"
#include "string_helper.h"

namespace live_object_explorer {

int string_resize_callback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        auto text = reinterpret_cast<std::string*>(data->UserData);
        IM_ASSERT(data->Buf == text->data());
        text->resize(data->BufTextLen);
        data->Buf = text->data();
    }
    return 0;
}

}  // namespace live_object_explorer
