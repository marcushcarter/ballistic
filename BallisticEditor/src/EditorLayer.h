#pragma once

#include <Ballistic.h>

namespace Ballistic {

	class EditorLayer : public Layer {
	public:
		EditorLayer(const std::string name = "EditorLayer") : Layer(name) {}

		void onAttach() override;
		void onDetach() override;
		void onUpdate() override;
		void onEvent(void* e) override;
	private:
		
	};
}