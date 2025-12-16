#pragma once

#include <Ballistic.h>
#include "Panels/IPanel.h"
#include "ImGuiContext.h"

namespace Ballistic {

	class EditorLayer : public Layer {
	public:
		EditorLayer(const LayerContext& context, std::shared_ptr<ImGuiContext> imguiContext, const std::string name = "EditorLayer");

		void onAttach() override;
		void onDetach() override;
		void onUpdate() override;
		void onEvent(void* e) override;

	private:
		std::shared_ptr<ProjectManager> m_ProjectManager;
		
		std::shared_ptr<LayerStack> m_LayerStack;

        std::shared_ptr<IWindow> m_Window;
        std::shared_ptr<OglRenderer> m_OglRenderer;

		std::shared_ptr<ImGuiContext> m_ImGuiContext;

        std::vector<std::unique_ptr<IPanel>> m_Panels;
	};
}