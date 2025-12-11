#include "LayerStack.h"

namespace Ballistic {

	void LayerStack::pushLayer(std::shared_ptr<Layer> layer) {
        m_Layers.push_back(layer);
        layer->onAttach();
	}

	void LayerStack::popLayer(std::shared_ptr<Layer> layer) {
        auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
        if (it != m_Layers.end()) {
            (*it)->onDetach();
            m_Layers.erase(it);
        }
	}

	void LayerStack::onUpdate() {
		for (auto& layer : m_Layers)
           	layer->onUpdate();
	}

	void LayerStack::onDetach() {
		for (auto& layer : m_Layers)
			layer->onDetach();
	}

	void LayerStack::dispatchEvent(void* e) {
		for (auto& layer : m_Layers)
            layer->onEvent(e);
	}

}