#pragma once
#include "pch.h"
#include "window.h"
#include "graphics/renderer.h"
#include "project/project.h"

struct Application
{
    Window window;
    Renderer renderer;
    
    std::future<void> loadFuture;
    std::atomic<bool> projectLoading{false};
    std::atomic<bool> projectDataReady{false};
    std::atomic<bool> projectLoadFailed{false};

    std::function<void()> onProjectLoadFailed;

    Project project;

    bool Create(const char* title, int width, int height);
    void Run();
    void Destroy();

    void OpenProject(const std::filesystem::path& path);
    void CloseProject();

    virtual void OnInit() {}
    virtual void OnUpdate() {}
    virtual void OnShutdown() {}

    virtual void OnProjectOpened(const std::filesystem::path& path) { (void)path; }
    virtual void OnProjectClosed() {}

    virtual ~Application() = default;
};