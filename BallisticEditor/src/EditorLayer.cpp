#include "EditorLayer.h"

#include "Panels/MenuBar/MenuBar.h"
#include "Panels/HierarchyPanel/HierarchyPanel.h"
#include "Panels/InspectorPanel/InspectorPanel.h"
#include "Panels/ViewportPanel/ViewportPanel.h"

namespace Ballistic {

	EditorLayer::EditorLayer(const LayerContext& context, const std::string name) : Layer(name) {
		m_projectManager = context.projectManager;
		m_window = context.window;
		m_renderer = context.renderer;
	}

	void EditorLayer::OnAttach() {
		m_panels.push_back(std::make_unique<MenuBar>(m_projectManager, m_window));
		m_panels.push_back(std::make_unique<HierarchyPanel>(m_projectManager));
		m_panels.push_back(std::make_unique<InspectorPanel>(m_projectManager));
		m_panels.push_back(std::make_unique<ViewportPanel>(m_renderer));

	    for (auto& panel : m_panels)
        	panel->Init();
	}

	void EditorLayer::OnDetach() {
	}

	void EditorLayer::OnUpdate() {
		if (m_window->HasImGuiContext()) {
			for (auto& panel : m_panels)
				panel->OnImGuiRender();

			ImGui::ShowDemoWindow();
		}
	}

	void EditorLayer::OnEvent(void* ePtr) {
		Event* e = static_cast<Event*>(ePtr);
		for (auto& panel : m_panels)
        	panel->OnImGuiRender();
	}
}