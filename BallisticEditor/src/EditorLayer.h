#pragma once

#include <Ballistic.h>

namespace Ballistic {

	class EditorLayer : public Layer {
	public:
		EditorLayer(LayerStack& layerStack, const std::string name = "EditorLayer")
			: Layer(name), m_LayerStack(&layerStack) {}

		void onAttach() override;
		void onDetach() override;
		void onUpdate() override;
		void onEvent(void* e) override;

	private:
		LayerStack* m_LayerStack = nullptr;
	};
}