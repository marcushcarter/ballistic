#include "application.h"

bool Application::Create(const char* title, int width, int height)
{
    LOG_INFO("%s v%d.%d.%d.stable.official.%s - https://ballisticgames.ca", APP_NAME, APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH, APP_COMMIT);

    window.Create(title, width, height);
    renderer.Start(window);   
    splash.Create(renderer);
    OnInit();
    
    window.Show();
    return true;
}

void Application::Run()
{
    while (!window.ShouldClose()) {
        window.PollEvents();
 
        if (state == AppState::LoadingProject) {
            TickLoading();
            continue;
        }

        OnUpdate();
        
        if (renderer.BeginFrame()) {
            renderer.RecordSwapchainPass(renderer.onSwapchainPass);
            renderer.EndFrame();
        }
    }
}

void Application::Destroy()
{
    OnShutdown();
    renderer.device.Wait();
    splash.Destroy();
    renderer.Shutdown();
    window.Destroy();
}

void Application::OpenProject(const std::filesystem::path& path)
{
    state = AppState::LoadingProject;
    loader.Begin(project, path);
}

void Application::TickLoading()
{
    switch (loader.Poll()) {
        case ProjectLoader::Status::Loading:
            splash.RenderLoadingFrame(renderer);
            return;
 
        case ProjectLoader::Status::Failed:
            loader.Reset();
            state = AppState::Active;
            CloseProject();
            if (onProjectLoadFailed) onProjectLoadFailed();
            return;
 
        case ProjectLoader::Status::Succeeded: {
            // CPU data is ready and the worker has produced it; create the GPU
            // mirror here on the main thread, which owns the device/queue.
            bool ok = renderer.resources.LoadProject(renderer, project);
            loader.Reset();
            state = AppState::Active;
 
            if (!ok) {
                LOG_ERROR("Failed to load project GPU resources: %s", project.path.string().c_str());
                CloseProject();
                if (onProjectLoadFailed) onProjectLoadFailed();
                return;
            }
 
            OnProjectOpened(project.path);
            return;
        }
 
        case ProjectLoader::Status::Idle:
        default:
            state = AppState::Active;
            return;
    }
}

void Application::CloseProject()
{
    OnProjectClosed();
    renderer.resources.DestroyAll();
    project.Close();
}
