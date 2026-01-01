#pragma once
#include "Panels/IPanel.h"
#include "imgui.h"

namespace ballistic
{
	class ConsolePanel : public IPanel {
	public:
        ConsolePanel(LayerContext& context, PanelStack& panelStack, const std::string& name = "Console");
        ~ConsolePanel() override { OnDetach(); }
        
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float deltaTime) override;
		void OnEvent(IEvent& e) override;

    private:
        char filterBuffer[128] = {};
        char commandBuffer[256] = {};
        bool scrollToBottom = true;
	};
}