#pragma once
#include <vk/vk.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/render_graph/render_path.h>
#include <functional>

struct Window;
struct Project;

struct Renderer
{
    bool windowResizeRequested = false;
    bool viewportResizeRequested = false;
    bool vsyncChangeRequested = false;
    uint32_t pendingWindowW = 0, pendingWindowH = 0;
    uint32_t pendingViewportW = 0, pendingViewportH = 0;
    bool pendingVSync = false;

    // CORE

    uint64_t frameNumber = 0;
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

    Image2D finalImage;
    Image2D logoImage;
    Image2D logoLongImage;
    Sampler linearSampler;

    BindlessHeap bindlessHeap;
    PipelineLayout globalPipelineLayout;

    PipelineCache pipelineCache;
    // GraphicsPipeline blitPipeline;

    RenderGraph graph;
    std::unique_ptr<RenderPath> renderPath;

    std::filesystem::path projectPath;

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

    bool Start(Window& window);
    void Shutdown();

    bool LoadProject(const std::filesystem::path& path);
    void UnloadProject();

    void RequestWindowResize(uint32_t w, uint32_t h);
    void RequestSceneResize(uint32_t w, uint32_t h);
    void RequestVSync(bool enabled);

    void WindowResize();
    void ViewportResize();
    void ApplyVSync();

    void SetRenderPath(std::unique_ptr<RenderPath> path) { renderPath = std::move(path); }

    void Render();

    bool BeginFrame();
    void EndFrame();
};