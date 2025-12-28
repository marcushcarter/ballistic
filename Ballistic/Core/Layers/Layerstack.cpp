#include "Core/Layers/Layerstack.h"
#include "Core/Layers/ILayer.h"
#include "Core/LogManager/Log.h"

namespace ballistic
{
    void LayerStack::PushLayer(std::shared_ptr<ILayer> layer) {
        m_layers.push_back(layer);
		LogDebug(layer->getName(), " layer attached");
        layer->OnAttach();
	}

	void LayerStack::PopLayer(std::shared_ptr<ILayer> layer) {
        auto it = std::find(m_layers.begin(), m_layers.end(), layer);
        if (it != m_layers.end()) {
			LogDebug((*it)->getName(), " layer dettached");
            (*it)->OnDetach();
            m_layers.erase(it);
        }
	}

	void LayerStack::OnUpdate(float deltaTime) {
		for (auto& layer : m_layers)
           	layer->OnUpdate(deltaTime);
	}

	void LayerStack::OnDetach() {
		for (auto& layer : m_layers)
			layer->OnDetach();
	}

	void LayerStack::DispatchEvent(IEvent& e) {
		for (auto& layer : m_layers)
            layer->OnEvent(e);
	}
    
} // namespace ballistic
