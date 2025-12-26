#pragma once
#include "bepch.h"

namespace Ballistic {

	class Layer;

	class LayerStack {
	public:
		LayerStack() = default;
		~LayerStack() = default;

		void PushLayer(std::shared_ptr<Layer> layer);
		void PopLayer(std::shared_ptr<Layer> layer);

		void OnUpdate();
		void OnDetach();
		void DispatchEvent(void* ePtr);

	public:
		std::vector<std::shared_ptr<Layer>> m_layers;
	};
}