#include "Panels/MenuPanel.h"

namespace ballistic
{
    MenuPanel::MenuPanel(LayerContext& context, const std::string& name) 
        : IPanel(context, name) {}
    
    void MenuPanel::OnAttach() {

    }
    
    void MenuPanel::OnDetach() {
    }

    void MenuPanel::OnUpdate(float deltaTime) {
        
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 12.0f));
        
        if (ImGui::BeginMainMenuBar()) {
            if (m_context.window->GetSettings().customTitleBar) {
                ImGui::TextUnformatted(m_context.window->GetSettings().title.c_str());
                ImGui::SameLine();
            }

            if (ImGui::BeginMenu("File")) {
                ImGui::MenuItem("New");
                ImGui::MenuItem("Open");
                ImGui::MenuItem("Save");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                ImGui::MenuItem("Undo");
                ImGui::MenuItem("Redo");
                ImGui::EndMenu();
            }
            
            const float buttonSize = ImGui::GetFrameHeight();
            const float windowWidth = ImGui::GetWindowWidth();

            float centerX = (windowWidth - buttonSize) * 0.5f;
            ImGui::SetCursorPosX(centerX);
            if (ImGui::Button((const char*)u8"\u25B6", ImVec2(buttonSize, buttonSize))) {}

            if (m_context.window->GetSettings().customTitleBar) {
                float totalWidth = buttonSize * 3;

                ImGui::SameLine(ImGui::GetWindowWidth() - totalWidth - ImGui::GetStyle().WindowPadding.x);

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

                if (ImGui::Button((const char*)u8"\u2013", ImVec2(buttonSize, buttonSize))) {}
                ImGui::SameLine();
                if (ImGui::Button((const char*)u8"\uF065", ImVec2(buttonSize, buttonSize))) {}
                ImGui::SameLine();
                if (ImGui::Button((const char*)u8"\u00D7", ImVec2(buttonSize, buttonSize))) {}

                ImGui::PopStyleVar();
            }
            
            ImGui::PopStyleVar();
            ImGui::EndMainMenuBar();
        }
    }

    void MenuPanel::OnEvent(IEvent& e) {

    }

} // namespace ballistic
