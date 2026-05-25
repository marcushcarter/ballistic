#pragma once
#include "pch.h"
#include "VK/VK.h"

struct Window;

struct Renderer
{
    bool windowResizeRequested = false;
    bool sceneResizeRequested = false;
    bool vsyncChangeRequested = false;
    uint32_t pendingWindowW = 0, pendingWindowH = 0;
    uint32_t pendingSceneW = 0, pendingSceneH = 0;
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

    bool Start(Window& window);
    bool CreateImGui(GLFWwindow* window);
    
    void Shutdown();
    void DestroyImGui();

    void RequestWindowResize(uint32_t w, uint32_t h);
    void RequestSceneResize(uint32_t w, uint32_t h);
    void RequestVSync(bool enabled);
    void WindowResize();
    void SceneResize();
    void ApplyVSync();

    bool BeginFrame();
    void EndFrame();
    bool RenderLoadingScreen();
};