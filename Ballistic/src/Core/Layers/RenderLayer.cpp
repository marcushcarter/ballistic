#include "Core/Layers/RenderLayer.h"
#include "Core/Layers/Event.h"
#include "Core/Layers/LayerStack.h"
#include "Core/Application.h"
#include "Renderer/Renderer.h"

namespace Ballistic {

	RenderLayer::RenderLayer(const LayerContext& context, const std::string name) : Layer(name) {
		m_layerStack = context.layerStack;
		m_renderer = context.renderer;
	}

	void RenderLayer::OnAttach() {
	}

	void RenderLayer::OnDetach() {
	}

	void RenderLayer::OnUpdate() {
		m_renderer->Render();
		
		// Event e(EventType::TestEvent, nullptr);
		// m_LayerStack->dispatchEvent(&e);
	}

	void RenderLayer::OnEvent(void* ePtr) {
		Event* e = static_cast<Event*>(ePtr);
	}
}