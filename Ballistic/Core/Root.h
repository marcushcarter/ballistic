#pragma once
#include "bepch.h"
#include "Singleton.h"
#include "Core/Logging/Logger.h"

namespace ballistic
{
    class IApplication;

    class Root : public Singleton<Root>
    {
    public:
        Root();
        ~Root();

        void SetApplication(std::unique_ptr<IApplication> app);

        bool Init();
        void Shutdown();
        void Run();

        void RequestShutdown() { m_running = false; }

        Logger* GetLogger() { return m_logger.get(); }
    
    private:
        std::unique_ptr<Logger> m_logger;
        
        bool m_running = false;
        std::unique_ptr<IApplication> m_app;
    };

    inline Root* GetRoot() { return Root::getSingletonPtr(); }
    
} // namespace ballistic
