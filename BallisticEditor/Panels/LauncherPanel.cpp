#include "Panels/LauncherPanel.h"
#include <tinyfiledialogs.h>

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
            ImGui::OpenPopup("NewProjectMenu");
        }

        if (ImGui::BeginPopup("NewProjectMenu")) {
            static char projectName[128] = "";
            static char projectPath[512] = "";
            
            ImGui::Text("Project Name:");
            ImGui::InputTextWithHint("##ProjectName", "New Project", projectName, IM_ARRAYSIZE(projectName));

            // ImGui::Spacing();

            ImGui::Text("Project Location:");
            ImGui::InputText("##ProjectPath", projectPath, IM_ARRAYSIZE(projectPath));

            std::string name = std::string(projectName);

            // ImGui::SameLine();
            if (ImGui::Button("Browse")) {
                const char* file = tinyfd_saveFileDialog("Select Project Location", (name.empty()) ? "New Project" : name.c_str(), 0, nullptr, nullptr);

                if (file && strlen(file) > 0) {
                    std::filesystem::path selectedFile(file);
                    std::filesystem::path folder = selectedFile.parent_path();

                    strncpy(projectPath, folder.string().c_str(), IM_ARRAYSIZE(projectPath));
                    projectPath[IM_ARRAYSIZE(projectPath) - 1] = '\0';
                }
            }

            // ImGui::Spacing();
            // ImGui::Dummy(ImVec2(0, 10));

            std::filesystem::path path(projectPath);
            bool canCreate =
                !path.empty() &&
                std::filesystem::exists(path) &&
                std::filesystem::is_directory(path);

            ImGui::BeginDisabled(!canCreate);
            if (ImGui::Button("Create Project")) {
                
                if (GetRoot()->GetProjectManager()->CreateNewProject(path, name))
                    m_panelStack.QueueAction(PanelStaction::OpenEditor);
                
                projectName[0] = '\0';
                projectPath[0] = '\0';
            }
            ImGui::EndDisabled();

            ImGui::EndPopup();
        }

        ImGui::SetCursorPosX((vp->WorkSize.x - 200) * 0.5f);

        if (ImGui::Button("Open Project", ImVec2(200, 40))) {
            const char* filters[] = { "project.config" };
            const char* file = tinyfd_openFileDialog("Open Project", "", 1, filters, "Project Config", 0);
            if (file && strlen(file) > 0) {
                std::filesystem::path configFile(file);
                if (GetRoot()->GetProjectManager()->OpenProject(configFile))
                    m_panelStack.QueueAction(PanelStaction::OpenEditor);
            }
        }

        ImGui::End();
    }

    void LauncherPanel::OnEvent(IEvent& e) {

    }

} // namespace ballistic
