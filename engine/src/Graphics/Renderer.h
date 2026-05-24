#pragma once
#include "pch.h"
#include "VK/VK.h"

struct Window;

struct Renderer
{
    // Window* window = nullptr;
    // uint32_t width = 1, height = 1;
    // float aspect = 1.0f;
    // bool resizeRequested = false;

    // CORE

    uint32_t frameCount = 0;
    uint32_t imageIndex = 0;
    uint32_t currentFrame = 0;

    Instance instance;
    DebugMessenger debugMessenger;
    Surface surface;
    PhysicalDevice physicalDevice;
    Device device;
    Queue graphicsQueue, presentQueue, transferQueue, computeQueue;
    bool hasAsyncCompute = false;

    CommandPool graphicsCommandPool;

    std::vector<CommandBuffer> commandBuffers;
    std::vector<Semaphore> imageAvailableSemaphores;
    std::vector<Semaphore> renderFinishedSemaphores;
    std::vector<Fence> inFlightFences;
    VkCommandBuffer cmd = VK_NULL_HANDLE;

    Swapchain swapchain;
    std::vector<Image2D> swapchainImages;
    
    Allocator allocator;

    DescriptorPool descriptorPool;
    DescriptorPool imguiDescriptorPool;

    Image2D finalImage;
    Image2D logoImage;
    Image2D logoLongImage;
    Sampler linearSampler;

    DescriptorSetLayout finalImageInputSetLayout;
    DescriptorSet finalImageInputSet;
    PipelineLayout blitPipelineLayout;
    GraphicsPipeline blitPipeline;

    // std::vector<Image2D> images;
    // std::vector<Sampler> samplers;
    // std::vector<RenderPass> renderPasses;
    // std::vector<Framebuffer> framebuffers;
    // std::vector<Buffer> buffers;
    // std::vector<DescriptorSetLayout> setLayouts;
    // std::vector<DescriptorSet> descriptorSets;
    // std::vector<PipelineLayout> pipelineLayouts;
    // std::vector<Semaphore> semaphores;
    // std::vector<Fence> fences;
    // std::vector<CommandBuffer> commandBuffers;

    std::function<void(VkCommandBuffer)> onSwapchainPass;
    
    std::string imguiIniPath;
    std::filesystem::path cacheRoot;

    bool Start(Window& window);
    bool CreateImGui(GLFWwindow* window, const std::string& iniPath);
    bool DeserializeScene(const std::string& path);
    
    void Shutdown();
    void DestroyImGui();

    bool BeginFrame();
    void EndFrame();
    
    // void SetCacheRoot(const std::filesystem::path& path) { cacheRoot = path; }
};