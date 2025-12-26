#include "Core/Layers/LayerStack.h"
#include "Core/Layers/Layer.h"

namespace Ballistic {

	void LayerStack::PushLayer(std::shared_ptr<Layer> layer) {
        m_layers.push_back(layer);
        layer->OnAttach();
	}

	void LayerStack::PopLayer(std::shared_ptr<Layer> layer) {
        auto it = std::find(m_layers.begin(), m_layers.end(), layer);
        if (it != m_layers.end()) {
            (*it)->OnDetach();
            m_layers.erase(it);
        }
	}

	void LayerStack::OnUpdate() {
		for (auto& layer : m_layers)
           	layer->OnUpdate();
	}

	void LayerStack::OnDetach() {
		for (auto& layer : m_layers)
			layer->OnDetach();
	}

	void LayerStack::DispatchEvent(void* ePtr) {
		for (auto& layer : m_layers)
            layer->OnEvent(ePtr);
	}

}