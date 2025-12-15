#include "DemoPanel.h"

#include <imgui.h>

namespace Ballistic {
	
	void DemoPanel::init() {
	}

	void DemoPanel::OnImGuiRender() {
		ImGui::Begin("Demo Panel");
        ImGui::Text("Hello from DemoPanel!");
        if (ImGui::Button("Click Me")) {}
        ImGui::End();
	}
	
	void DemoPanel::onEvent(void* e) {
	}
}