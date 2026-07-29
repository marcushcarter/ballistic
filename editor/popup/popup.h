#pragma once
#include <editor/editor_context.h>
#include <imgui.h>

namespace ballistic {

struct Popup
{
    bool open = false;

    virtual ~Popup() = default;
    virtual const char* name() const = 0;
    virtual void draw_contents(EditorContext& ctx) = 0;

    void draw(EditorContext& ctx)
    {
        if (open) {
            ImGui::OpenPopup(name());
            open = false;
        }

        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 250), ImGuiCond_Appearing);

        if (!ImGui::BeginPopupModal(name(), nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings)) return;

        const float footer_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().WindowPadding.y;

        ImGui::BeginChild("##popup_body", ImVec2(0, -footer_h));
        draw_contents(ctx);
        ImGui::EndChild();
        const float bw = 120.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - bw) * 0.5f);
        if (ImGui::Button("Close", ImVec2(bw, 0))) { open = false; ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }
};

}