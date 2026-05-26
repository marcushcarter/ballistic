#include "application.h"

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

void Application::OpenProject(const std::filesystem::path& path)
{
    projectLoading = true;

    loadFuture = std::async(std::launch::async, [this, path]() {
        projectLoadFailed = !DeserializeProject(path);
        projectLoading = false;
    });

    while (projectLoading && !window.ShouldClose()) {
        window.PollEvents();
        renderer.RenderLoadingScreen();
    }

    if (projectLoadFailed) {
        projectLoadFailed = false;
        CloseProject();
        if (onProjectLoadFailed) onProjectLoadFailed();
        LOG_ERROR("Failed to open project: %s", path.string().c_str());
    } else {
        projectPath = path;
        OnProjectOpened(path);
    }
}

bool Application::DeserializeProject(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        LOG_ERROR("Project path does not exist: %s", path.string().c_str());
        return false;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    return true;
}
 
void Application::CloseProject()
{
    // destroy vulkan and scene stuff
    OnProjectClosed();
    LOG_INFO("PROJECT CLOSED");
}
