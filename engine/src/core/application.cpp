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
    projectDataReady = false;
    projectLoadFailed = false;

    loadFuture = std::async(std::launch::async, [this, path]()
    {
        if (!std::filesystem::exists(path)) {
            LOG_ERROR("Project path does not exist: %s", path.string().c_str());
            projectLoadFailed = true;
            projectLoading = false;
            return;
        }

        if (!project.Load(path)) {
            LOG_ERROR("Failed to deserialize project: %s", path.string().c_str());
            projectLoadFailed = true;
            projectLoading = false;
            return;
        }
        
        projectDataReady = true;
    });

    while (projectLoading && !window.ShouldClose()) {
        window.PollEvents();

        if (projectDataReady.exchange(false)) {
            if (!renderer.LoadProject(project)) {
                LOG_ERROR("Failed to load project Vulkan resources: %s", path.string().c_str());
                projectLoadFailed = true;
            }
            projectLoading = false;
        }

        renderer.RenderLoadingScreen();
    }

    if (projectLoadFailed) {
        projectLoadFailed = false;
        CloseProject();
        if (onProjectLoadFailed) onProjectLoadFailed();
    } else {
        OnProjectOpened(project.path);
    }
}

void Application::CloseProject()
{
    OnProjectClosed();
    renderer.UnloadProject();
    project.Close();
}
