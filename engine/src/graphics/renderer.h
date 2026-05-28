#pragma once
#include "pch.h"
#include "vk/vk.h"
#include "render_graph_resources.h"

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

    Image2D finalImage;
    Sampler linearSampler;

    DescriptorSetLayout imageInputSetLayout;
    DescriptorSet finalImageInputSet;
    PipelineLayout blitPipelineLayout;
    GraphicsPipeline blitPipeline;

    RenderGraphResources resources;

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

    std::function<void()> onViewportResized; // pretty much only for updatign the imgui final texture descriptor set

    std::function<void(VkCommandBuffer)> onSwapchainPass;

    bool Start(Window& window);
    void Shutdown();

    void RequestWindowResize(uint32_t w, uint32_t h);
    void RequestSceneResize(uint32_t w, uint32_t h);
    void RequestVSync(bool enabled);

    void WindowResize();
    void ViewportResize();
    void ApplyVSync();

    bool BeginFrame();
    void RecordSwapchainPass(const std::function<void(VkCommandBuffer)>& content);
    void EndFrame();
};