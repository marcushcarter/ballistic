#include "Application.h"

bool Application::Create(const char* title, int width, int height)
{
    LOG_INFO("%s v%d.%d.%d.stable.official.%s - https://ballisticgames.ca", APP_NAME, APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH, APP_COMMIT);
    
    window.Create(title, width, height);
    renderer.Create(window);
    
    return true;
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
    renderer.Destroy();
    window.Destroy();
}
