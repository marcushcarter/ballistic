#include <editor/popup/confirm/confirm.h>
#include <imgui.h>

namespace ballistic {

void ConfirmPopup::draw_contents(EditorContext&)
{
    if (!title.empty()) {
        ImGui::TextUnformatted(title.c_str());
        ImGui::Separator();
        ImGui::Spacing();
    }
    ImGui::TextWrapped("%s", message.c_str());
}

void ConfirmPopup::draw_footer(EditorContext&)
{
    const char* labels[] = { confirm_label.c_str(), "Cancel" };
    switch (footer_buttons(labels, 2)) {
        case 0:
            if (on_confirm) on_confirm();
            close();
            break;
        case 1:
            close();
            break;
    }
}

}