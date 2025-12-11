#include "RuntimeLayer.h"

namespace Ballistic {

	void RuntimeLayer::onAttach() {
	}

	void RuntimeLayer::onDetach() {
	}

	void RuntimeLayer::onUpdate() {
	}

	void RuntimeLayer::onEvent(void* ePtr) {
		Event* e = static_cast<Event*>(ePtr);
	}
}