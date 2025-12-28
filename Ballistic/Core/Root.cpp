#include "Core/Root.h"
#include "Core/IApplication.h"
#include "Core/LogManager/Log.h"

namespace ballistic
{
    void Root::SetApplication(std::unique_ptr<IApplication> app) {
        m_app = std::move(app);
    }

    bool Root::Init() {
        m_logger = std::make_unique<LogManager>();
        if (!m_logger->Init()) {
            return false;
        }

        if (!m_app->Init()) {
            return false;
        }

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

        if (m_logger) {
            m_logger->Shutdown();
        }
        
        m_running = false;
    }
    
} // namespace ballistic
