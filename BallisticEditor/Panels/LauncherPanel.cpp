#include "Panels/LauncherPanel.h"

namespace ballistic
{
    LauncherPanel::LauncherPanel(LayerContext& context, PanelStack& panelStack, const std::string& name) 
        : IPanel(context, panelStack, name) {}
    
    void LauncherPanel::OnAttach() {
    }
    
    void LauncherPanel::OnDetach() {
    }

    void LauncherPanel::OnUpdate(float deltaTime) {
        ImGuiViewport* vp = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGui::Begin("Launcher", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoDocking);

        ImGui::Dummy(ImVec2(0, vp->WorkSize.y * 0.35f));

        ImGui::SetCursorPosX((vp->WorkSize.x - 200) * 0.5f);

        if (ImGui::Button("New Project", ImVec2(200, 40))) {
            // CreateNewProject();

            m_panelStack.QueueAction(PanelStaction::OpenEditor);
        }

        ImGui::SetCursorPosX((vp->WorkSize.x - 200) * 0.5f);

        if (ImGui::Button("Open Project", ImVec2(200, 40))) {
            // OpenProject();
        }

        ImGui::End();
    }

    void LauncherPanel::OnEvent(IEvent& e) {

    }

} // namespace ballistic
