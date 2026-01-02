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

        m_renderer = std::make_unique<Renderer>();
        m_renderer->ApplyWindowHints();

        m_window = std::make_unique<Window>();
        if (!m_window->Init(settings))
            return false;

        if (!m_renderer->Init(m_window.get()))
            return false;

        if (!GetRoot()->GetMeshManager()->Init())
            return false;

        std::vector<Vertex> cubeVertices = {
            {{-0.5f, -0.5f,  0.5f}, {0,0,1}, {0,0}},
            {{ 0.5f, -0.5f,  0.5f}, {0,0,1}, {1,0}},
            {{ 0.5f,  0.5f,  0.5f}, {0,0,1}, {1,1}},
            {{-0.5f,  0.5f,  0.5f}, {0,0,1}, {0,1}},
            {{-0.5f, -0.5f, -0.5f}, {0,0,-1}, {1,0}},
            {{ 0.5f, -0.5f, -0.5f}, {0,0,-1}, {0,0}},
            {{ 0.5f,  0.5f, -0.5f}, {0,0,-1}, {0,1}},
            {{-0.5f,  0.5f, -0.5f}, {0,0,-1}, {1,1}},
            {{-0.5f, -0.5f, -0.5f}, {-1,0,0}, {0,0}},
            {{-0.5f, -0.5f,  0.5f}, {-1,0,0}, {1,0}},
            {{-0.5f,  0.5f,  0.5f}, {-1,0,0}, {1,1}},
            {{-0.5f,  0.5f, -0.5f}, {-1,0,0}, {0,1}},
            {{ 0.5f, -0.5f, -0.5f}, {1,0,0}, {1,0}},
            {{ 0.5f, -0.5f,  0.5f}, {1,0,0}, {0,0}},
            {{ 0.5f,  0.5f,  0.5f}, {1,0,0}, {0,1}},
            {{ 0.5f,  0.5f, -0.5f}, {1,0,0}, {1,1}},
            {{-0.5f,  0.5f,  0.5f}, {0,1,0}, {0,0}},
            {{ 0.5f,  0.5f,  0.5f}, {0,1,0}, {1,0}},
            {{ 0.5f,  0.5f, -0.5f}, {0,1,0}, {1,1}},
            {{-0.5f,  0.5f, -0.5f}, {0,1,0}, {0,1}},
            {{-0.5f, -0.5f,  0.5f}, {0,-1,0}, {0,1}},
            {{ 0.5f, -0.5f,  0.5f}, {0,-1,0}, {1,1}},
            {{ 0.5f, -0.5f, -0.5f}, {0,-1,0}, {1,0}},
            {{-0.5f, -0.5f, -0.5f}, {0,-1,0}, {0,0}},
        };

        std::vector<uint32_t> cubeIndices = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            8, 9,10,10,11, 8,
            12,13,14,14,15,12,
            16,17,18,18,19,16,
            20,21,22,22,23,20
        };

        GetRoot()->GetMeshManager()->AddMesh("Cube Mesh", cubeVertices, cubeIndices);
        GetRoot()->GetMeshManager()->LoadMesh(IApplication::GetResDirectory() / "Models/stanford_dragon_pbr/scene.gltf");
        
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
