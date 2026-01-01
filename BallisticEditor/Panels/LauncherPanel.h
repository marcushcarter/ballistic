#pragma once
#include "Panels/IPanel.h"
#include "imgui.h"

namespace ballistic
{
	class LauncherPanel : public IPanel {
	public:
        LauncherPanel(LayerContext& context, PanelStack& panelStack, const std::string& name = "Launcher");
        ~LauncherPanel() override { OnDetach(); }
        
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float deltaTime) override;
		void OnEvent(IEvent& e) override;
	};
}