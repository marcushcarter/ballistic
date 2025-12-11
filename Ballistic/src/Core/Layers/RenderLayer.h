#pragma once

#include "Layer.h"
#include "LayerStack.h"

namespace Ballistic {

	class RenderLayer : public Layer {
	public:
		RenderLayer(LayerStack& layerStack, const std::string name = "RenderLayer")
			: Layer(name), m_LayerStack(&layerStack) {}

		void onAttach() override;
		void onDetach() override;
		void onUpdate() override;
		void onEvent(void* e) override;

	private:
		LayerStack* m_LayerStack = nullptr;
	};
}