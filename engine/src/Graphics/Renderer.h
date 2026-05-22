#pragma once
#include "pch.h"
#include "VK/VK.h"

struct Window;

struct Renderer
{
    Instance instance;
    DebugMessenger debugMessenger;
    Surface surface;
    PhysicalDevice physicalDevice;
    Device device;
    Queue graphicsQueue, presentQueue, transferQueue, computeQueue;
    bool hasAsyncCompute = false;
    uint32_t frameCount = 0;

    bool Create(Window& window);
    void Destroy();
    void Render();
};