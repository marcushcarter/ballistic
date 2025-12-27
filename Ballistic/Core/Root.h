#pragma once
#include "bepch.h"
#include "Singleton.h"

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
    
    private:
        bool m_running = false;
        std::unique_ptr<IApplication> m_app;
    };
    
} // namespace ballistic

#define GetRoot() ballistic::Root::getSingletonPtr()
