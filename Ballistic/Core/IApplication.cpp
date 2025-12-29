#include "Core/IApplication.h"
#include "Core/Layers/LayerStack.h"
#include "Core/Window/Window.h"
#include "Renderer/Renderer.h"
#include "Root/Root.h"

namespace ballistic
{   
    bool IApplication::Init() {
        m_layerStack = std::make_shared<LayerStack>();
        
        WindowSettings settings = GetWindowSettings();

        m_renderer = std::make_unique<Renderer>(RendererAPI::OpenGL);
        m_renderer->ApplyWindowHints();

        m_window = std::make_unique<Window>();
        if (!m_window->Init(settings)) return false;

        if (!m_renderer->Init(m_window.get())) return false;
        
        m_layerContext = CreateLayerContext();

        return OnInit();
    }

    void IApplication::Update(float deltaTime) {
        if (m_window->ShouldClose()) {
            GetRoot()->RequestShutdown();
            return;
        }

        m_layerStack->OnUpdate(deltaTime);
        
        m_renderer->OnUpdate();
        
        OnUpdate(deltaTime);

        m_window->Update(deltaTime);
    }

    void IApplication::Shutdown() {
        OnShutdown();

        m_renderer->Shutdown();

        m_window->Shutdown();

        m_layerStack->OnDetach();
    }

} // namespace ballistic
