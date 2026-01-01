#pragma once
#include "Panels/IPanel.h"
#include "imgui.h"

namespace ballistic
{
	class MenuPanel : public IPanel {
	public:
        MenuPanel(LayerContext& context, PanelStack& panelStack, PanelStaction type = PanelStaction::OpenEditor, const std::string& name = "Menu");
        ~MenuPanel() override { OnDetach(); }
        
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float deltaTime) override;
		void OnEvent(IEvent& e) override;
		
		void OpenLink(const std::string& url);

	private:
		PanelStaction m_type;
	};
}