#include "RuntimeLayer.h"

namespace Ballistic {

	void RuntimeLayer::onAttach() {
		std::cout << "Runtime Layer Attached" << std::endl;
	}

	void RuntimeLayer::onDetach() {
	}

	void RuntimeLayer::onUpdate() {
	}

	void RuntimeLayer::onEvent(void* ePtr) {
		Event* e = static_cast<Event*>(ePtr);
		if (e->getType() == EventType::TestEvent) {
			std::cout << "Event passed from Render Layer to Runtime Layer" << std::endl;
	    }
	}
}