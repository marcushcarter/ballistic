#pragma once
#include "bepch.h"

namespace ballistic
{
	class ILayer;
    class IEvent;

	class LayerStack {
	public:
		LayerStack() = default;
		~LayerStack() { Clear(); }

		void PushLayer(std::shared_ptr<ILayer> layer);
		void PopLayer(std::shared_ptr<ILayer> layer);

		void OnUpdate(float deltaTime);
		void Clear();
		void DispatchEvent(IEvent& e);

	public:
		std::vector<std::shared_ptr<ILayer>> m_layers;
	};
    
} // namespace ballistic