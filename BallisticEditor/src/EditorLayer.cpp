#include "EditorLayer.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "Panels/Dockspace/Dockspace.h"
#include "Panels/MenuBar/MenuBar.h"
#include "Panels/HierarchyPanel/HierarchyPanel.h"
#include "Panels/InspectorPanel/InspectorPanel.h"
#include "Panels/ViewportPanel/ViewportPanel.h"

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

		m_Panels.push_back(std::make_unique<Dockspace>());

		m_Panels.push_back(std::make_unique<MenuBar>(m_ProjectManager, m_Window));
		m_Panels.push_back(std::make_unique<HierarchyPanel>(m_ProjectManager));
		m_Panels.push_back(std::make_unique<InspectorPanel>(m_ProjectManager));
		m_Panels.push_back(std::make_unique<ViewportPanel>(m_OglRenderer));

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

	    for (auto& panel : m_Panels)
        	panel->OnImGuiRender();

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