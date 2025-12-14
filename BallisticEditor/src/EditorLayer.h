#pragma once

#include <Ballistic.h>
#include "Panels/IPanel.h"

namespace Ballistic {

	class EditorLayer : public Layer {
	public:
		EditorLayer(const LayerContext& context, const std::string name = "EditorLayer")
			: Layer(name), m_LayerStack(context.layerStack), m_Window(context.window), m_VkRenderer(context.renderer) {}

		void onAttach() override;
		void onDetach() override;
		void onUpdate() override;
		void onEvent(void* e) override;

		void GenDockspace();
		void DrawMenuBar();

	private:
		LayerStack* m_LayerStack = nullptr;

        IWindow* m_Window = nullptr;
        VkRenderer* m_VkRenderer = nullptr;

        std::vector<std::unique_ptr<IPanel>> m_Panels;
	};
}