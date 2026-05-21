#pragma once
#include "pch.h"
#include "window/window.h"

struct Application
{
    Window window;

    bool Create(const char* title, int width, int height);
    void Run();
    void Destroy();

protected:
    virtual void OnInit() {}
    virtual void OnUpdate() {}
    virtual void OnShutdown() {}
};