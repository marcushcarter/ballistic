#pragma once
#include "pch.h"
#include "window.h"
#include "graphics/renderer.h"
#include "graphics/splash_renderer.h"
#include "project/project.h"
#include "project/project_loader.h"

enum class AppState { Active, LoadingProject };

struct Application
{
    Window window;
    Renderer renderer;
    SplashRenderer splash;

    Project project;
    ProjectLoader loader;

    AppState state = AppState::Active;

    std::function<void()> onProjectLoadFailed;

    bool Create(const char* title, int width, int height);
    void Run();
    void Destroy();

    void OpenProject(const std::filesystem::path& path);
    void TickLoading();
    void CloseProject();


    virtual void OnInit() {}
    virtual void OnUpdate() {}
    virtual void OnShutdown() {}

    virtual void OnProjectOpened(const std::filesystem::path& path) { (void)path; }
    virtual void OnProjectClosed() {}

    virtual ~Application() = default;
};