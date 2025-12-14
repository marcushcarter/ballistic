#pragma once

#include <Ballistic.h>

namespace Ballistic {

	class RuntimeLayer : public Layer {
	public:
		RuntimeLayer(const LayerContext& context, const std::string name = "RuntimeLayer")
			: Layer(name), m_LayerStack(context.layerStack), m_Window(context.window), m_VkRenderer(context.renderer) {}

		void onAttach() override;
		void onDetach() override;
		void onUpdate() override;
		void onEvent(void* e) override;

	private:
		LayerStack* m_LayerStack = nullptr;
		
        IWindow* m_Window = nullptr;
        VkRenderer* m_VkRenderer = nullptr;
	};
}