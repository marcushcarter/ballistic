#pragma once
#include "pch.h"
#include "vk/vk.h"

struct Window;
struct Project;
struct RGImage;

struct Renderer
{
    bool windowResizeRequested = false;
    bool viewportResizeRequested = false;
    bool vsyncChangeRequested = false;
    uint32_t pendingWindowW = 0, pendingWindowH = 0;
    uint32_t pendingViewportW = 0, pendingViewportH = 0;
    bool pendingVSync = false;

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

    DescriptorSetLayout imageInputSetLayout;
    DescriptorSet finalImageInputSet;
    PipelineLayout blitPipelineLayout;
    GraphicsPipeline blitPipeline;
    
    DescriptorSet splashSet;
    PipelineLayout splashPipelineLayout;
    GraphicsPipeline splashPipeline;

    struct SplashPushConstants {
        float x, y, w, h;
    };

    struct AllocatedImage {
        Image2D image;
        bool viewportRelative = false;
        float relativeWidth = 1.0f;
        float relativeHeight = 1.0f;
    };
    std::unordered_map<uint64_t, AllocatedImage> allocatedImages;

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

    std::function<void()> onViewportResized;

    std::function<void(VkCommandBuffer)> onSwapchainPass;

    bool Start(Window& window);
    bool CreateImGui(GLFWwindow* window);
    bool LoadProject(const Project& project);
    
    void Shutdown();
    void DestroyImGui();
    void UnloadProject();
    
    bool RecreateImage(const RGImage& desc);
    void DestroyImage(uint64_t id);

    void RequestWindowResize(uint32_t w, uint32_t h);
    void RequestSceneResize(uint32_t w, uint32_t h);
    void RequestVSync(bool enabled);

    void WindowResize();
    void ViewportResize();
    void ApplyVSync();

    bool BeginFrame();
    void EndFrame();
    bool RenderLoadingScreen();
};