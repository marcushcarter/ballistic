#include "Core/IApplication.h"
#include "Core/Window/Window.h"
#include "Renderer/Renderer.h"
#include "Scene/SceneManager.h"
#include "Root/Root.h"

#include "Root/LogManager/Log.h"

namespace ballistic
{   
    bool IApplication::Init() {

        m_sceneManager = std::make_unique<SceneManager>();
        if (!m_sceneManager->Init())
            return false;
        
        WindowSettings settings = GetWindowSettings();

        m_renderer = std::make_unique<Renderer>(RendererAPI::OpenGL);
        m_renderer->ApplyWindowHints();

        m_window = std::make_unique<Window>();
        if (!m_window->Init(settings))
            return false;

        if (!m_renderer->Init(m_window.get()))
            return false;
        
        m_layerContext = CreateLayerContext();

        return OnInit();
    }

    void IApplication::Update(float deltaTime) {
        if (m_window->ShouldClose()) {
            GetRoot()->RequestShutdown();
            return;
        }
        BeginFrame();

        OnUpdate(deltaTime);
        m_layerStack.OnUpdate(deltaTime);
        m_renderer->Render(m_sceneManager->GetActiveScene());

        EndFrame();
        m_window->Present();
    }

    void IApplication::Shutdown() {
        OnShutdown();

        m_layerStack.Clear();
        m_renderer->Shutdown();
        m_window->Shutdown();
        m_sceneManager->Shutdown();
    }

} // namespace ballistic
