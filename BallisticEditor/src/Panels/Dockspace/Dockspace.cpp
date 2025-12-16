#include "Dockspace.h"

namespace Ballistic {
	
	void Dockspace::init() {
	}

	void Dockspace::OnImGuiRender() {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
	    ImGui::SetNextWindowPos(viewport->Pos);
	    ImGui::SetNextWindowSize(viewport->Size);
	    ImGui::SetNextWindowViewport(viewport->ID);

	    ImGuiWindowFlags window_flags =
	        ImGuiWindowFlags_NoDocking |
	        ImGuiWindowFlags_NoTitleBar |
	        ImGuiWindowFlags_NoCollapse |
	        ImGuiWindowFlags_NoResize |
	        ImGuiWindowFlags_NoMove |
	        ImGuiWindowFlags_NoBringToFrontOnFocus |
	        ImGuiWindowFlags_NoNavFocus |
	        ImGuiWindowFlags_MenuBar;

	    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	    ImGui::Begin("dockspace", nullptr, window_flags);
	    
	    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
	    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
	    
	    ImGui::PopStyleVar(3);
	    ImGui::End();
	}
	
	void Dockspace::onEvent(void* e) {
	}
}