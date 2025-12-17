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

	EditorLayer::EditorLayer(const LayerContext& context, std::shared_ptr<ImGuiContext> imguiContext, const std::string name) : Layer(name) {
		m_ProjectManager = context.projectManager;
		m_LayerStack = context.layerStack;
		m_Window = context.window;
		m_OglRenderer = context.renderer;

		m_ImGuiContext = imguiContext;
	}

	void EditorLayer::onAttach() {
		m_Panels.push_back(std::make_unique<Dockspace>());

		m_Panels.push_back(std::make_unique<MenuBar>(m_ProjectManager, m_Window));
		m_Panels.push_back(std::make_unique<HierarchyPanel>(m_ProjectManager));
		m_Panels.push_back(std::make_unique<InspectorPanel>(m_ProjectManager));
		m_Panels.push_back(std::make_unique<ViewportPanel>(m_OglRenderer));

	    for (auto& panel : m_Panels)
        	panel->init();
	}

	void EditorLayer::onDetach() {
	}

	void EditorLayer::onUpdate() {
		m_ImGuiContext->BeginFrame();

	    for (auto& panel : m_Panels)
        	panel->OnImGuiRender();

	    ImGui::ShowDemoWindow();

		m_ImGuiContext->EndFrame();
	}

	void EditorLayer::onEvent(void* ePtr) {
		Event* e = static_cast<Event*>(ePtr);
	}
}