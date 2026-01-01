#include "Panels/PanelStack.h"
#include "Panels/IPanel.h"
#include "Panels/MenuPanel.h"
#include "Panels/LauncherPanel.h"
#include "Panels/DemoPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ConsolePanel.h"

namespace ballistic
{
	PanelStack::PanelStack(LayerContext& layerContext) 
		: m_context(layerContext) {}

    void PanelStack::PushPanel(std::unique_ptr<IPanel> panel) {
		LogDebug(panel->getName(), " panel attached");
        panel->OnAttach();
        m_panels.push_back(std::move(panel));
	}

	void PanelStack::PopPanel(IPanel* panel) {
		auto it = std::find_if(
			m_panels.begin(),
			m_panels.end(),
			[panel](const std::unique_ptr<IPanel>& p) { return p.get() == panel; }
		);

		if (it != m_panels.end()) {
			LogDebug((*it)->getName(), " panel detached");
			(*it)->OnDetach();
			m_panels.erase(it);
		}
	}

	void PanelStack::OnUpdate(float deltaTime) {
		for (auto& panel : m_panels)
           	panel->OnUpdate(deltaTime);

		ExecutePendingAction();
	}

	void PanelStack::Clear() {
		for (auto& panel : m_panels)
			panel->OnDetach();

		m_panels.clear();
	}

	void PanelStack::DispatchEvent(IEvent& e) {
		for (auto& panel : m_panels)
            panel->OnEvent(e);
	}
	
	void PanelStack::OpenEditor(LayerContext& context) {
		Clear();

		PushPanel(std::make_unique<MenuPanel>(context, *this));
		PushPanel(std::make_unique<ViewportPanel>(context, *this));
		PushPanel(std::make_unique<HierarchyPanel>(context, *this));
		PushPanel(std::make_unique<InspectorPanel>(context, *this));
		PushPanel(std::make_unique<ConsolePanel>(context, *this));
		// PushPanel(std::make_unique<DemoPanel>(m_context, *this));
	}

	void PanelStack::OpenLauncher(LayerContext& context) {
		Clear();

		PushPanel(std::make_unique<MenuPanel>(context, *this, PanelStaction::OpenLauncher));
		PushPanel(std::make_unique<LauncherPanel>(context, *this));
	}
    
	void PanelStack::ExecutePendingAction() {
		switch (m_pendingAction) {
			case PanelStaction::OpenEditor: OpenEditor(m_context); break;
			case PanelStaction::OpenLauncher: OpenLauncher(m_context); break;
			default: break;
		}
		m_pendingAction = PanelStaction::None;
	}

} // namespace ballistic
