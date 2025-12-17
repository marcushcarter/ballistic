#pragma once

#include <Ballistic.h>
#include "Panels/IPanel.h"

namespace Ballistic {

	class InspectorPanel : public IPanel {
	public:
        InspectorPanel(std::shared_ptr<ProjectManager> projectManager);
        
		void init() override;
		void OnImGuiRender() override;
		void onEvent(void* e) override;
    
    private:
        std::shared_ptr<ProjectManager> m_ProjectManager;

		template<typename T>
		inline void DrawComponent(const std::string& title, EntityHandle& entity, std::function<void()> contentFunc = {}, bool deletable = true) {
        	ImGui::PushID(title.c_str());
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
			ImGui::BeginChild("TransformChildWindow", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_FrameStyle);

			bool treeOpen = ImGui::TreeNodeEx(
				title.c_str(),
				ImGuiTreeNodeFlags_DefaultOpen |
				ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_Framed |
				ImGuiTreeNodeFlags_FramePadding
			);

			if (treeOpen) {
				ImGui::BeginChild(
					"TransformFields",
					ImVec2(ImGui::GetContentRegionAvail().x - 5.0f, 0),
					ImGuiChildFlags_AutoResizeY
				);

				// if (deletable) {
				// 	if (ImGui::Button("X")) entity.remove<T>();
				// }

				if (contentFunc) contentFunc();

				ImGui::EndChild();
				ImGui::TreePop();
				ImGui::Spacing();
			}

			ImGui::EndChild();
			ImGui::PopStyleVar();
			ImGui::PopID();
		}
	};
}