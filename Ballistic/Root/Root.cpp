#include "Root/Root.h"
#include "Core/IApplication.h"
#include "Root/LogManager/Log.h"

namespace ballistic
{
    void Root::SetApplication(std::unique_ptr<IApplication> app) {
        m_app = std::move(app);
    }

    bool Root::Init() {
        m_logManager = std::make_unique<LogManager>();
        if (!m_logManager->Init())
            return false;
            
		LogInfo("Ballistic Engine ", BALLISTIC_ENGINE_VERSION, " (c) 2025-present Marcus Carter.");
        
        m_meshManager = std::make_unique<MeshManager>();
        if (!m_meshManager->Init())
            return false;

        // std::vector<Vertex> cubeVertices = {
        //     // Front face
        //     {{-0.5f, -0.5f,  0.5f}, {0,0,1}, {0,0}},
        //     {{ 0.5f, -0.5f,  0.5f}, {0,0,1}, {1,0}},
        //     {{ 0.5f,  0.5f,  0.5f}, {0,0,1}, {1,1}},
        //     {{-0.5f,  0.5f,  0.5f}, {0,0,1}, {0,1}},

        //     // Back face
        //     {{-0.5f, -0.5f, -0.5f}, {0,0,-1}, {1,0}},
        //     {{ 0.5f, -0.5f, -0.5f}, {0,0,-1}, {0,0}},
        //     {{ 0.5f,  0.5f, -0.5f}, {0,0,-1}, {0,1}},
        //     {{-0.5f,  0.5f, -0.5f}, {0,0,-1}, {1,1}},

        //     // Left face
        //     {{-0.5f, -0.5f, -0.5f}, {-1,0,0}, {0,0}},
        //     {{-0.5f, -0.5f,  0.5f}, {-1,0,0}, {1,0}},
        //     {{-0.5f,  0.5f,  0.5f}, {-1,0,0}, {1,1}},
        //     {{-0.5f,  0.5f, -0.5f}, {-1,0,0}, {0,1}},

        //     // Right face
        //     {{ 0.5f, -0.5f, -0.5f}, {1,0,0}, {1,0}},
        //     {{ 0.5f, -0.5f,  0.5f}, {1,0,0}, {0,0}},
        //     {{ 0.5f,  0.5f,  0.5f}, {1,0,0}, {0,1}},
        //     {{ 0.5f,  0.5f, -0.5f}, {1,0,0}, {1,1}},

        //     // Top face
        //     {{-0.5f,  0.5f,  0.5f}, {0,1,0}, {0,0}},
        //     {{ 0.5f,  0.5f,  0.5f}, {0,1,0}, {1,0}},
        //     {{ 0.5f,  0.5f, -0.5f}, {0,1,0}, {1,1}},
        //     {{-0.5f,  0.5f, -0.5f}, {0,1,0}, {0,1}},

        //     // Bottom face
        //     {{-0.5f, -0.5f,  0.5f}, {0,-1,0}, {0,1}},
        //     {{ 0.5f, -0.5f,  0.5f}, {0,-1,0}, {1,1}},
        //     {{ 0.5f, -0.5f, -0.5f}, {0,-1,0}, {1,0}},
        //     {{-0.5f, -0.5f, -0.5f}, {0,-1,0}, {0,0}},
        // };

        // std::vector<uint32_t> cubeIndices = {
        //     // Front
        //     0, 1, 2, 2, 3, 0,
        //     // Back
        //     4, 5, 6, 6, 7, 4,
        //     // Left
        //     8, 9,10,10,11, 8,
        //     // Right
        //     12,13,14,14,15,12,
        //     // Top
        //     16,17,18,18,19,16,
        //     // Bottom
        //     20,21,22,22,23,20
        // };

        // m_meshManager->AddMesh("Cube Mesh", cubeVertices, cubeIndices);
        // m_meshManager->AddMesh("Cube Mesh2", cubeVertices, cubeIndices);
        // m_meshManager->AddMesh("Cube Mesh3", cubeVertices, cubeIndices);

        if (!m_app->Init())
            return false;
            
        LogDebug("Root initialized");
        return true;
    }

    void Root::Run() {
        if (!m_app) {
            return;
        }
        m_running = true;

        using clock = std::chrono::high_resolution_clock;
        auto lastTime = clock::now();

        while (m_running) {
            auto currentTime = clock::now();
            std::chrono::duration<float> elapsed = currentTime - lastTime;
            lastTime = currentTime;
            float deltaTime = elapsed.count();

            m_app->Update(deltaTime);
        }
    }

    void Root::Shutdown() {
        if (m_app) {
            m_app->Shutdown();
        }

        if (m_meshManager) {
            m_meshManager->Shutdown();
        }

        if (m_logManager) {
            m_logManager->Shutdown();
        }
        
        m_running = false;
    }
    
} // namespace ballistic
