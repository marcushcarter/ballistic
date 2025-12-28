#pragma once
#include "Panels/IPanel.h"
#include "imgui.h"

namespace ballistic
{
	class DemoPanel : public IPanel {
	public:
        DemoPanel(LayerContext context, const std::string& name = "Demo") 
            : IPanel(context, name) {}
        ~DemoPanel() override { OnDetach(); }
        
		void OnAttach() override {
        }
        
		void OnDetach() override {
        }

		void OnUpdate(float deltaTime) override {
            ImGui::ShowDemoWindow();
        }

		void OnEvent(IEvent& e) override {
        }
	};
}