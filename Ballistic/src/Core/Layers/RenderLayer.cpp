#include "RenderLayer.h"

#include "Event.h"

namespace Ballistic {

	void RenderLayer::onAttach() {
	}

	void RenderLayer::onDetach() {
	}

	void RenderLayer::onUpdate() {
	}

	void RenderLayer::onEvent(void* ePtr) {
		Event* e = static_cast<Event*>(ePtr);
	}
}