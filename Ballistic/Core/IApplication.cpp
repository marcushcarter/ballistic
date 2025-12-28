#include "Core/IApplication.h"
#include "Core/Layers/LayerStack.h"
#include "Core/Window/Window.h"

namespace ballistic
{
    IApplication::IApplication() {
        // m_layerStack = std::make_shared<LayerStack>();
    }
    
    bool IApplication::Init() {
        m_layerStack = std::make_shared<LayerStack>();
        m_window = std::make_unique<Window>();

        m_layerContext = CreateLayerContext();

        if (!OnInit())
            return false;

        return true;
    }

    void IApplication::Update(float deltaTime) {
        m_layerStack->OnUpdate(deltaTime);

        OnUpdate(deltaTime);
    }

    void IApplication::Shutdown() {
        m_layerStack->OnDetach();

        OnShutdown();
    }

} // namespace ballistic
