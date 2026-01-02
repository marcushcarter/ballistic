#include "Panels/MenuPanel.h"

namespace ballistic
{
	void MenuPanel::OpenLink(const std::string& url) {
	#ifdef _WIN32
		ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	#elif __APPLE__
		std::string cmd = "open " + url;
		std::system(cmd.c_str());
	#else // Linux/Unix
		std::string cmd = "xdg-open " + url;
		std::system(cmd.c_str());
	#endif
	}

    MenuPanel::MenuPanel(LayerContext& context, PanelStack& panelStack, PanelStaction type, const std::string& name) 
        : IPanel(context, panelStack, name), m_type(type) {}
    
    void MenuPanel::OnAttach() {
    }
    
    void MenuPanel::OnDetach() {
    }

    void MenuPanel::OnUpdate(float deltaTime) {
        
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 12.0f));
        
        if (ImGui::BeginMainMenuBar()) {

            if (m_type == PanelStaction::OpenEditor) {
                if (ImGui::BeginMenu("Project")) {
                    if (ImGui::MenuItem("New Scene")) {}
                    if (ImGui::MenuItem("Open Scene")) {}
                    if (ImGui::MenuItem("Open Recent")) {}
                    ImGui::Separator();
                    if (ImGui::MenuItem("Save")) {}
                    if (ImGui::MenuItem("Save As")) {}
                    if (ImGui::MenuItem("Save Copy")) {}
                    if (ImGui::MenuItem("Save Increments")) {}
                    ImGui::Separator();
                    if (ImGui::MenuItem("Set Project Directory")) {}
                    if (ImGui::MenuItem("Import")) {}
                    // if (ImGui::MenuItem("Export")) {}
                    ImGui::Separator();
                    if (ImGui::MenuItem("Undo")) {}
                    if (ImGui::MenuItem("Redo")) {}
                    if (ImGui::MenuItem("Undo History")) {}
                    ImGui::Separator();
                    if (ImGui::MenuItem("Quit")) {}
                    ImGui::EndMenu();
                }

                // if (ImGui::BeginMenu("Debug")) {
                // 	ImGui::EndMenu();
                // }

                if (ImGui::BeginMenu("Editor")) {
                    if (ImGui::MenuItem("Editor Settings")) {}
                    if (ImGui::MenuItem("Editor Colors")) {}
                    ImGui::Separator();
                    if (ImGui::MenuItem("Take Editor Screenshot")) {}
                    if (ImGui::MenuItem("Take Viewport Screenshot")) {}
                    if (ImGui::MenuItem(m_context.window->GetState().fullscreen ? "Windowed" : "Fullscreen")) m_context.window->ToggleFullscreen(!m_context.window->GetState().fullscreen);
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Help")) {
                    if (ImGui::MenuItem("Search Help")) {}
                    ImGui::Separator();
                    if (ImGui::MenuItem("Online Documentation")) OpenLink("https://ballisticstudios.ca/");
                    if (ImGui::MenuItem("Forums")) OpenLink("https://ballisticstudios.ca/");
                    if (ImGui::MenuItem("Community")) OpenLink("https://ballisticstudios.ca/");
                    if (ImGui::MenuItem("Tutorials")) OpenLink("https://ballisticstudios.ca/");
                    ImGui::Separator();
                    if (ImGui::MenuItem("Report a bug")) OpenLink("https://ballisticstudios.ca/");
                    ImGui::Separator();
                    if (ImGui::MenuItem("About The Engine")) OpenLink("https://ballisticstudios.ca/");
                    if (ImGui::MenuItem("Support Development")) OpenLink("https://ballisticstudios.ca/");
                    ImGui::EndMenu();
                }
                
                const float buttonSize = ImGui::GetFrameHeight();
                const float windowWidth = ImGui::GetWindowWidth();

                float centerX = (windowWidth - buttonSize) * 0.5f;
                ImGui::SetCursorPosX(centerX);
                if (ImGui::Button((const char*)u8"\u25B6", ImVec2(buttonSize, buttonSize))) {}

                std::string projectText = GetRoot()->GetProjectManager()->GetName() + " - ";

                std::string sceneText = (m_context.sceneManager->HasActiveScene())
                    ? (!m_context.sceneManager->GetActiveScene()->GetName().empty()
                        ? m_context.sceneManager->GetActiveScene()->GetName() + " - "
                        : "Unnamed Scene - ")
                    : "" ;

                std::string nodeText = "";
                if (m_context.sceneManager->HasActiveScene()) {
                    Scene* scene = m_context.sceneManager->GetActiveScene();
                    entt::entity selected = scene->GetSelected();

                    if (selected != entt::null) {
                        auto& registry = scene->GetRegistry();

                        if (registry.all_of<Tag>(selected)) {
                            auto& tag = registry.get<Tag>(selected);
                            nodeText = !tag.name.empty() ? tag.name : "Unnamed Node";
                            nodeText += " - ";
                        } else {
                            nodeText = "Unnamed Node - ";
                        }
                    }
                }

                std::string text = nodeText + sceneText + projectText + "Ballistic Engine";
                float textWidth = ImGui::CalcTextSize(text.c_str()).x;
                float extraPadding = 20.0f;

                ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - textWidth - ImGui::GetStyle().FramePadding.x - extraPadding);
                ImGui::TextUnformatted(text.c_str());

            } else if (m_type == PanelStaction::OpenLauncher) {

            }
            
            ImGui::PopStyleVar();
            ImGui::EndMainMenuBar();
        }
    }

    void MenuPanel::OnEvent(IEvent& e) {

    }

} // namespace ballistic
