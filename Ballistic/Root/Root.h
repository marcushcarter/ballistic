#pragma once
#include "bepch.h"
#include "Singleton.h"
#include "Root/LogManager/LogManager.h"
#include "Root/MeshManager/MeshManager.h"
#include "Root/ProjectManager/ProjectManager.h"
#include "Core/IApplication.h"

#define BALLISTIC_ENGINE_VERSION "v1.0"

namespace ballistic
{
    class IApplication;

    class Root : public Singleton<Root>
    {
    public:
        Root() = default;
        ~Root() { Shutdown(); }

        void SetApplication(std::unique_ptr<IApplication> app);

        bool Init();
        void Shutdown();
        void Run();

        void RequestShutdown() { m_running = false; }

        LogManager* GetLogManager() { return m_logManager.get(); }
        MeshManager* GetMeshManager() { return m_meshManager.get(); }
        ProjectManager* GetProjectManager() { return m_projectManager.get(); }
    
    private:
        std::unique_ptr<LogManager> m_logManager;
        std::unique_ptr<MeshManager> m_meshManager;
        std::unique_ptr<ProjectManager> m_projectManager;
        
        bool m_running = false;
        std::unique_ptr<IApplication> m_app;
    };

    inline Root* GetRoot() { return Root::getSingletonPtr(); }
    
} // namespace ballistic
