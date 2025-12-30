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
