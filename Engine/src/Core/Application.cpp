#include "Application.h"

bool Application::Create(const char* title, int width, int height)
{
    LOG_INFO("%s v%d.%d.%d.stable.official.%s - https://ballisticgames.ca", APP_NAME, APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH, APP_COMMIT);
    
    window.Create(title, width, height);
    renderer.Start(window);   
    OnInit();
    
    window.Show();
    return true;
}

void Application::Run()
{
    while (!window.ShouldClose()) {
        window.PollEvents();
        OnUpdate();

        if (renderer.BeginFrame()) {
            renderer.EndFrame();
        }
    }
}

void Application::Destroy()
{
    OnShutdown();
    renderer.Shutdown();
    window.Destroy();
}
