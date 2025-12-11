#include "RenderLayer.h"

#include "Event.h"

namespace Ballistic {

	void RenderLayer::onAttach() {
		std::cout << "Render Layer Attached" << std::endl;
	}

	void RenderLayer::onDetach() {
	}

	void RenderLayer::onUpdate() {
		Event e(EventType::TestEvent, nullptr);
		m_LayerStack->dispatchEvent(&e);
	}

	void RenderLayer::onEvent(void* ePtr) {
		Event* e = static_cast<Event*>(ePtr);
	}
}