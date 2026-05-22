#include "Application.h"

bool Application::Create(const char* title, int width, int height)
{
    bool result = window.Create(title, width, height);
    if (!result) {
        std::cout << "stupid fucker" << std::endl;
    }
    
    return result;
}

void Application::Run()
{
    OnInit();

    while (!window.ShouldClose()) {
        window.PollEvents();
        OnUpdate();
    }

    OnShutdown();
}

void Application::Destroy()
{
    window.Destroy();
}
