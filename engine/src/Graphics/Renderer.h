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

    Swapchain swapchain;
    std::vector<Image2D> swapchainImages;
    RenderPass swapchainRenderPass;
    std::vector<Framebuffer> swapchainFramebuffers;

    CommandPool commandPool;
    CommandPool transferCommandPool;

    bool Create(Window& window);
    void Destroy();
    void Render();
};