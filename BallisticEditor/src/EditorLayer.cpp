#include "EditorLayer.h"

namespace Ballistic {

	void EditorLayer::onAttach() {
		std::cout << "Editor Layer Attached" << std::endl;
	}

	void EditorLayer::onDetach() {
	}

	void EditorLayer::onUpdate() {
	}

	void EditorLayer::onEvent(void* ePtr) {
		Event* e = static_cast<Event*>(ePtr);
	}
}