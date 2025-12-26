#pragma once
#include <Ballistic.h>

namespace Ballistic {

	class RuntimeLayer : public Layer {
	public:
		RuntimeLayer(const LayerContext& context, const std::string name = "RuntimeLayer");

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(void* ePtr) override;

	private:		
        std::shared_ptr<Window> m_window;
        std::shared_ptr<Renderer> m_renderer;
	};
}