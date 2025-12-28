#pragma once
#include "Panels/IPanel.h"
#include "imgui.h"

namespace ballistic
{
	class MenuPanel : public IPanel {
	public:
        MenuPanel(LayerContext& context, const std::string& name = "Menu");
        ~MenuPanel() override { OnDetach(); }
        
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float deltaTime) override;
		void OnEvent(IEvent& e) override;
	};
}