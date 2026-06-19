#pragma once
#include <ballistic_vulkan.h>
#include <window/window.h>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>

namespace Ballistic::Launcher
{
struct Project
{
    std::string name;
    std::string path;
    std::string engineVersion;
    std::string lastOpened;
    bool favorite = false;
};

struct LauncherApplication
{
    Window window;
    bool viewportResized = false;

    std::chrono::high_resolution_clock::time_point lastFrameTime;
    float targetFPS = 120.0f;

    uint32_t frameCount = 0;
    uint32_t imageIndex = 0;
    uint32_t currentFrame = 0;

    Vulkan::Instance instance;
    Vulkan::Surface surface;
    Vulkan::PhysicalDevice physicalDevice;
    Vulkan::Device device;
    Vulkan::Queue graphicsQueue;
    Vulkan::Queue presentQueue;

    Vulkan::CommandPool commandPool;

    std::vector<Vulkan::CommandBuffer> commandBuffers;
    std::vector<Vulkan::Semaphore> imageAvailable;
    std::vector<Vulkan::Semaphore> renderFinished;
    std::vector<Vulkan::Fence> inFlight;

    Vulkan::Swapchain swapchain;
    std::vector<Vulkan::Image2D> swapchainImages;

    Vulkan::DescriptorPool descriptorPool;

    std::vector<Project> projects;

    char filterBuffer[256] = {};
    char renameBuffer[256] = {};
    int selectedIndex = -1;
    int sortIndex = 0;
    int removeConfirmIndex = -1;
    int renameIndex = -1;
    
    bool m_Running = true;

    void Run();
    void Close();

    void Setup();
    void Shutdown();

    void RequestResize();
    void RecreateSwapchain();

    VkCommandBuffer BeginFrame();
    void Render(VkCommandBuffer cmd);
    void EndFrame(VkCommandBuffer cmd);

    void BuildLauncherUI();
    void DrawList();
};
}