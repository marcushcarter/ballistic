#pragma once
#include "pch.h"
#include "Window.h"
#include "Graphics/Renderer.h"

struct Application
{
    Window window;
    Renderer renderer;

    bool Create(const char* title, int width, int height);
    void Run();
    void Destroy();

    virtual void OnInit() {}
    virtual void OnUpdate() {}
    virtual void OnShutdown() {}

    virtual ~Application() = default;
};