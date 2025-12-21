#pragma once
#include <Ballistic.h>
#include "Panels/IPanel.h"
#include "ImGuiContext.h"

namespace Ballistic {

	class EditorLayer : public Layer {
	public:
		EditorLayer(const LayerContext& context, std::shared_ptr<ImGuiContext> imguiContext, const std::string name = "EditorLayer");

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(void* ePtr) override;

	private:
		std::shared_ptr<ProjectManager> m_projectManager;
        std::shared_ptr<Window> m_window;
        std::shared_ptr<IRenderer> m_renderer;

		std::shared_ptr<ImGuiContext> m_imguiContext;

        std::vector<std::unique_ptr<IPanel>> m_panels;
	};
}