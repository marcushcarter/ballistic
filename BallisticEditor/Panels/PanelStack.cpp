#include "Panels/PanelStack.h"
#include "Panels/IPanel.h"

namespace ballistic
{
    void PanelStack::PushPanel(std::shared_ptr<IPanel> panel) {
        m_panels.push_back(panel);
		LogDebug(panel->getName(), " panel attached");
        panel->OnAttach();
	}

	void PanelStack::PopPanel(std::shared_ptr<IPanel> panel) {
        auto it = std::find(m_panels.begin(), m_panels.end(), panel);
        if (it != m_panels.end()) {
			LogDebug((*it)->getName(), " panel dettached");
            (*it)->OnDetach();
            m_panels.erase(it);
        }
	}

	void PanelStack::OnUpdate(float deltaTime) {
		for (auto& panel : m_panels)
           	panel->OnUpdate(deltaTime);
	}

	void PanelStack::OnDetach() {
		for (auto& panel : m_panels)
			panel->OnDetach();
	}
    
} // namespace ballistic
