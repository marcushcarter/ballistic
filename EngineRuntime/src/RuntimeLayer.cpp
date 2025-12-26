#include "RuntimeLayer.h"

namespace Ballistic {

	RuntimeLayer::RuntimeLayer(const LayerContext& context, const std::string name) : Layer(name) {
		m_window = context.window;
		m_renderer = context.renderer;
	}

	void RuntimeLayer::OnAttach() {
	}

	void RuntimeLayer::OnDetach() {
	}

	void RuntimeLayer::OnUpdate() {
		auto windowState = m_window->GetState();
		m_renderer->RequestResize(glm::vec2(windowState.width, windowState.height));
		// resize camera

		m_renderer->GetDevice()->BlitToScreen();
	}

	void RuntimeLayer::OnEvent(void* ePtr) {
		Event* e = static_cast<Event*>(ePtr);
		// if (e->getType() == EventType::TestEvent) {
		// 	std::cout << "Event passed from Render Layer to Runtime Layer" << std::endl;
	    // }
	}
}