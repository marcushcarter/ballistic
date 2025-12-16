#include "EditorLayer.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "Panels/DemoPanel/DemoPanel.h"
#include "Panels/HierarchyPanel/HierarchyPanel.h"

namespace Ballistic {

	EditorLayer::EditorLayer(const LayerContext& context, const std::string name) : Layer(name) {
		m_ProjectManager = context.projectManager;
		m_LayerStack = context.layerStack;
		m_Window = context.window;
		m_OglRenderer = context.renderer;
	}

	void EditorLayer::onAttach() {
		IMGUI_CHECKVERSION();
	    ImGui::CreateContext();

	    ImGuiIO& io = ImGui::GetIO();
	    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    	ImGui::StyleColorsDark();

    	switch (WindowAPI::GetAPI()) {
	    	case WindowAPI::API::GLFW:
	    		ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(m_Window->get()), true);
	    		break;

	    	default:
	    		break;
    	}
		
	    ImGui_ImplOpenGL3_Init("#version 460");

		m_Panels.push_back(std::make_unique<HierarchyPanel>(m_ProjectManager));

	    for (auto& panel : m_Panels)
        	panel->init();
	}

	void EditorLayer::onDetach() {
		
	    ImGui_ImplOpenGL3_Shutdown();

    	switch (WindowAPI::GetAPI()) {
	    	case WindowAPI::API::GLFW:
	    		ImGui_ImplGlfw_Shutdown();
	    		break;
	    	default:
	    		break;
    	}

    	ImGui::DestroyContext();
	}

	void EditorLayer::GenDockspace() {
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

	void EditorLayer::DrawMenuBar() {
		// entt::entity& selected = editorState.temp.selectedEntity;

		if (ImGui::BeginMainMenuBar()) {

			if (ImGui::BeginMenu("Project")) {

				if (ImGui::MenuItem("New Scene")) {
					// manager.Clear();
					// scene.clear();
					// selected = entt::null;
				}
				if (ImGui::MenuItem("Open Scene")) {}
				if (ImGui::MenuItem("Open Recent")) {}
				
				ImGui::Separator();

				if (ImGui::MenuItem("Save")) {}
				if (ImGui::MenuItem("Save As")) {}
				if (ImGui::MenuItem("Save Copy")) {}
				if (ImGui::MenuItem("Save Increments")) {}

				ImGui::Separator();

				if (ImGui::MenuItem("Set Project Directory")) {}
				if (ImGui::MenuItem("Import")) {

					// const char* filters[] = { "*.obj" };
					// const char* files = tinyfd_openFileDialog("Pick files", "", 1, filters, "OBJ Files", 1);

					// if (files && *files != '\0') {
					//     std::string filesStr = files;
					//     size_t start = 0, end;

					//     do {
					//         end = filesStr.find("|", start);
					//         std::string path = filesStr.substr(start, end - start);

					//         if (!path.empty()) {
					//             std::filesystem::path p(path);
					//             std::string ext = p.extension().string();
					//             std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

					//             if (ext == ".obj") {
					//                 manager.AddRtxMesh(p.stem().string(), path);
					//             }
					//         }

					//         start = (end == std::string::npos) ? filesStr.size() : end + 1;
					//     } while (start < filesStr.size());
					// }
				}

				// if (ImGui::MenuItem("Export")) {}

				ImGui::Separator();

				if (ImGui::MenuItem("Undo")) {}
				if (ImGui::MenuItem("Redo")) {}
				if (ImGui::MenuItem("Undo History")) {}

				ImGui::Separator();

				if (ImGui::MenuItem("Quit")) { glfwTerminate(); }

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Debug")) {

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Editor")) {

				if (ImGui::MenuItem("Editor Settings")) {}
				if (ImGui::MenuItem("Editor Colors")) {}

				ImGui::Separator();

				// if (ImGui::MenuItem("Take Editor Screenshot")) {} // takeScreenshotNextFrame = true;
				// if (ImGui::MenuItem("Take Viewport Screenshot")) {} // takeTextureScreenshotNextFrame = true;

				if (ImGui::MenuItem(m_Window->isFullscreen() ? "Windowed" : "Fullscreen")) m_Window->toggleFullscreen(!m_Window->isFullscreen());

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help")) {

				if (ImGui::MenuItem("Search Help")) {}

				ImGui::Separator();

				if (ImGui::MenuItem("Online Documentation")) {} // OpenLink("https://ballisticstudios.ca/");
				if (ImGui::MenuItem("Forums")) {} // OpenLink("https://ballisticstudios.ca/");
				if (ImGui::MenuItem("Community")) {} // OpenLink("https://ballisticstudios.ca/");
				if (ImGui::MenuItem("Tutorials")) {} // OpenLink("https://ballisticstudios.ca/");

				ImGui::Separator();

				if (ImGui::MenuItem("Report a bug")) {} // OpenLink("https://ballisticstudios.ca/");

				ImGui::Separator();

				if (ImGui::MenuItem("About The Engine")) {} // OpenLink("https://ballisticstudios.ca/");
				if (ImGui::MenuItem("Support Development")) {} // OpenLink("https://ballisticstudios.ca/");

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void EditorLayer::onUpdate() {
		
	    ImGui_ImplOpenGL3_NewFrame();

    	switch (WindowAPI::GetAPI()) {
	    	case WindowAPI::API::GLFW:
	    		ImGui_ImplGlfw_NewFrame();
	    		break;
	    	default:
	    		break;
    	}

    	ImGui::NewFrame();

    	GenDockspace();
	    DrawMenuBar();

	    for (auto& panel : m_Panels)
        	panel->OnImGuiRender();

    	// ImGui::Begin("Editor Panel");
	    // ImGui::Text("Hello from Ballistic EditorLayer!");

	    // ImGui::End();

	    ImGui::ShowDemoWindow();

    	ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    	auto io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		    if (WindowAPI::GetAPI() == WindowAPI::API::GLFW) {
		        GLFWwindow* backup = static_cast<GLFWwindow*>(m_Window->get());
		        ImGui::UpdatePlatformWindows();
		        ImGui::RenderPlatformWindowsDefault();
		        glfwMakeContextCurrent(backup);
		    }
		}
	}

	void EditorLayer::onEvent(void* ePtr) {
		Event* e = static_cast<Event*>(ePtr);
	}
}