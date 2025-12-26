#include "Core/Window/ImGuiSystem.h"
#include "Core/Config.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Ballistic {

    ImGuiSystem::~ImGuiSystem() {
		Shutdown();
    }

    void ImGuiSystem::Init(GLFWwindow* window, bool viewports, bool dockspace) {
		m_viewports = viewports;
		m_dockspace = dockspace;

		IMGUI_CHECKVERSION();
		m_context = ImGui::CreateContext();
    	ImGui::SetCurrentContext(m_context);

	    ImGuiIO& io = ImGui::GetIO();
	    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	    if (m_dockspace) io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    	if (m_viewports) io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImFontConfig cfg;
		cfg.PixelSnapH = true;
		cfg.MergeMode = false;
		io.Fonts->AddFontFromFileTTF((Config::RESOURCES_PATH / "Fonts/Noto_Sans/static/NotoSans-Regular.ttf").string().c_str(), 16.0f, &cfg);
		
		cfg.MergeMode = true;
		static const ImWchar icons_ranges[] = { 0xF000, 0xF8FF, 0 };
		io.Fonts->AddFontFromFileTTF((Config::RESOURCES_PATH / "Fonts/fontawesome-free-6.6.0-desktop/Font Awesome 6 Free-Solid-900.otf").string().c_str(), 13.0f, &cfg, icons_ranges);
		
    	ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            // style.WindowRounding = 0.0f;
            // style.TabRounding = 0.0f;
            // style.TabBarBorderSize = 0.0f;
            // style.GrabRounding = 2.0f;
            // style.ScrollbarRounding = 2.0f;
            // style.DockingSeparatorSize = 0.0f;
            // style.WindowBorderSize = 0.0f;
        }

        // style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        // style.WindowMenuButtonPosition = ImGuiDir_None;

	    ImGui_ImplGlfw_InitForOpenGL(window, true);
	    ImGui_ImplOpenGL3_Init("#version 460");
    }
    
    void ImGuiSystem::BeginFrame() {
		ImGui::SetCurrentContext(m_context);
	    ImGui_ImplOpenGL3_NewFrame();
	    ImGui_ImplGlfw_NewFrame();
    	ImGui::NewFrame();

		if (m_dockspace) {
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);

			ImGuiWindowFlags window_flags =
				ImGuiWindowFlags_NoDocking |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_NoNavFocus |
				ImGuiWindowFlags_MenuBar;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("dockspace", nullptr, window_flags);
			
			ImGuiID dockspace_id = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
			
			ImGui::PopStyleVar(3);
			ImGui::End();
		}
    }

    void ImGuiSystem::EndFrame() {
    	ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    	auto io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup);
		}
    }

    void ImGuiSystem::Shutdown() {
    	ImGui::SetCurrentContext(m_context);
		ImGui_ImplOpenGL3_Shutdown();
    	ImGui_ImplGlfw_Shutdown();
		if (m_context) {
			ImGui::DestroyContext(m_context);
			m_context = nullptr;
		}
    	ImGui::SetCurrentContext(nullptr);
    }
    
}