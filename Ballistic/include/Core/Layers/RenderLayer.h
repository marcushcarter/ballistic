#pragma once
#include "bepch.h"
#include "Layer.h"

namespace Ballistic {

	struct LayerContext;
	class LayerStack;
	class Renderer;

	class RenderLayer : public Layer {
	public:
		RenderLayer(const LayerContext& context, const std::string name = "RenderLayer");

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(void* ePtr) override;

	private:
		std::shared_ptr<LayerStack> m_layerStack;
        std::shared_ptr<Renderer> m_renderer;
	};
}