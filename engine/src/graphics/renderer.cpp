#include "renderer.h"
#include "core/window.h"
#include "project/project.h"
#include "shaders.h"
#include "resources.h"
#include "render_graph/render_path.h"

bool Renderer::Start(Window& window)
{
    uint32_t glfwExtCount = 0;
    const char** glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
    std::vector<const char*> instanceRequiredExtensions;
    instanceRequiredExtensions.assign(glfwExt, glfwExt + glfwExtCount);
    #if !defined(NDEBUG)
        instanceRequiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    #endif

    BE_ASSERT(instance.Create(APP_NAME, APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH, instanceRequiredExtensions));
    BE_ASSERT(debugMessenger.Create(instance.Get(), VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT));
    BE_ASSERT(surface.Create(instance.Get(), window.glfwWindow));
    
    std::vector<const char*> deviceExts = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    BE_ASSERT(physicalDevice.Pick(instance.Get(), surface.Get(), deviceExts));
    BE_ASSERT(device.Create(physicalDevice.Get(), physicalDevice.graphicsFamily, physicalDevice.presentFamily, physicalDevice.transferFamily, deviceExts));
    BE_ASSERT(graphicsQueue.Acquire(device.Get(), physicalDevice.graphicsFamily));
    BE_ASSERT(presentQueue.Acquire(device.Get(), physicalDevice.presentFamily));
    BE_ASSERT(transferQueue.Acquire(device.Get(), physicalDevice.transferFamily));
    BE_ASSERT(computeQueue.Acquire(device.Get(), physicalDevice.computeFamily));
    hasAsyncCompute = physicalDevice.HasDedicatedCompute();

    BE_ASSERT(swapchain.Create(physicalDevice.Get(), device.Get(), surface.Get(), { window.width, window.height }, false));
    std::vector<VkImage> rawImages = swapchain.GetImages();
    frameCount = static_cast<uint32_t>(rawImages.size());
    
    swapchainImages.resize(frameCount);
    for (uint32_t i = 0; i < frameCount; i++)
        BE_ASSERT(swapchainImages[i].WrapSwapchainImage(device.Get(), rawImages[i], swapchain.format, swapchain.extent));
    
    BE_ASSERT(graphicsCommandPool.Create(device.Get(), {
        .queueFamilyIndex = graphicsQueue.familyIndex,
        .resetable = true,
        .debugName = "GraphicsCommandPool"
    }));
    
    commandBuffers.resize(frameCount);
    imageAvailableSemaphores.resize(frameCount);
    renderFinishedSemaphores.resize(frameCount);
    inFlightFences.resize(frameCount);

    for (uint32_t i = 0; i < frameCount; i++) {
        BE_ASSERT(commandBuffers[i].Allocate(device.Get(), graphicsCommandPool.Get(), false, "CommandBuffer"));
        BE_ASSERT(imageAvailableSemaphores[i].Create(device.Get(), "ImageAvailableSemaphore"));
        BE_ASSERT(renderFinishedSemaphores[i].Create(device.Get(), "RenderFinishedSemaphore"));
        BE_ASSERT(inFlightFences[i].Create(device.Get(), true, "InFlightFence"));
    }
    
    BE_ASSERT(allocator.Create(instance.Get(), physicalDevice.Get(), device.Get()));

    graph.Init(device.Get(), allocator.Get());
    frameNumber = frameCount;

    BE_ASSERT(descriptorPool.Create(device.Get(), {
        .samplers = 1000,
        .combinedImageSamplers = 1000,
        .sampledImages = 1000,
        .storageImages = 1000,
        .uniformTexelBuffers = 1000,
        .storageTexelBuffers = 1000,
        .uniformBuffers = 1000,
        .storageBuffers = 1000,
        .uniformBuffersDynamic = 1000,
        .storageBuffersDynamic = 1000,
        .inputAttachments = 1000,
        .debugName = "MainDescriptorPool"
    }));

    BE_ASSERT(finalImage.Create(device.Get(), physicalDevice.memory, {
        .extent = swapchain.extent,
        .format = VK_FORMAT_R16G16B16A16_SFLOAT,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
        .debugName = "FinalImage"
    }));

    graph.SetViewport(finalImage.extent);
    
    BE_ASSERT(linearSampler.Create(device.Get(), {
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .debugName  = "LinearSampler"
    }));

    BE_ASSERT(imageInputSetLayout.Create(device.Get(), {
        .bindings = { SetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) },
        .debugName = "FinalImageInputSetLayout"
    }));

    BE_ASSERT(finalImageInputSet.Allocate(device.Get(), {
        .pool = descriptorPool.Get(),
        .setLayout = imageInputSetLayout.Get(),
        .debugName = "FinalImageInputSet"
    }));

    BE_ASSERT(blitPipelineLayout.Create(device.Get(), {
        .setLayouts = { imageInputSetLayout.Get() },
        .debugName = "BlitPipelineLayout"
    }));

    Shader vert{}, frag{};
    BE_ASSERT(vert.Compile(device.Get(), VK_SHADER_STAGE_VERTEX_BIT, { SHADER_FULLSCREEN_VERT, SHADER_FULLSCREEN_VERT + SHADER_FULLSCREEN_VERT_SIZE / 4 }));    
    BE_ASSERT(frag.Compile(device.Get(), VK_SHADER_STAGE_FRAGMENT_BIT, { SHADER_BLIT_FRAG, SHADER_BLIT_FRAG + SHADER_BLIT_FRAG_SIZE / 4 }));

    PipelineRenderingInfo renderingInfo;
    renderingInfo.colorFormats = { swapchain.format };
    auto renderingCreateInfo = renderingInfo.Get();

    BE_ASSERT(blitPipeline.Create(device.Get(), {
        .pNext = &renderingCreateInfo,
        .layout = blitPipelineLayout.Get(),
        .shaderStages = { PipelineShaderStage(vert.Get(), vert.stage), PipelineShaderStage(frag.Get(), frag.stage) },
        .debugName = "BlitPipeline"
    }));

    vert.Destroy();
    frag.Destroy();

    finalImageInputSet.SetImages(0, { finalImage.GetView() }, linearSampler.Get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    LOG_DEBUG("Renderer started");
    return true;
}

void Renderer::Shutdown()
{
    device.Wait();

    blitPipeline.Destroy();
    blitPipelineLayout.Destroy();
    imageInputSetLayout.Destroy();

    linearSampler.Destroy();
    finalImage.Destroy();
    graph.Shutdown();

    descriptorPool.Destroy();
    allocator.Destroy();

    for (uint32_t i = 0; i < frameCount; i++) {
        inFlightFences[i].Destroy();
        renderFinishedSemaphores[i].Destroy();
        imageAvailableSemaphores[i].Destroy();
        commandBuffers[i].Free();
    }
    graphicsCommandPool.Destroy();

    for (auto& img : swapchainImages) img.Destroy();
    swapchain.Destroy();

    device.Destroy();
    surface.Destroy();
    debugMessenger.Destroy();
    instance.Destroy();
    
    LOG_DEBUG("Renderer shutdown");
}

void Renderer::RequestWindowResize(uint32_t w, uint32_t h)
{
    if (w == 0 || h == 0) return;
    pendingWindowW = w;
    pendingWindowH = h;
    windowResizeRequested = true;
}

void Renderer::RequestSceneResize(uint32_t w, uint32_t h)
{
    if (w == 0 || h == 0) return;
    pendingViewportW = w;
    pendingViewportH = h;
    viewportResizeRequested = true;
}

void Renderer::RequestVSync(bool enabled)
{
    if (enabled == swapchain.vsync) return;
    pendingVSync = enabled;
    vsyncChangeRequested = true;
}

void Renderer::WindowResize()
{
    device.Wait();
    swapchain.Resize({ pendingWindowW, pendingWindowH }, false);
    std::vector<VkImage> rawImages = swapchain.GetImages();
    for (uint32_t i = 0; i < frameCount; i++)
        swapchainImages[i].WrapSwapchainImage(device.Get(), rawImages[i], swapchain.format, swapchain.extent);
    windowResizeRequested = false;
}

void Renderer::ViewportResize()
{
    device.Wait();
    finalImage.Resize({ pendingViewportW, pendingViewportH });
    graph.SetViewport(finalImage.extent);
    finalImageInputSet.SetImages(0, { finalImage.GetView() }, linearSampler.Get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    if (onViewportResized) onViewportResized();
    viewportResizeRequested = false;
}

void Renderer::ApplyVSync()
{
    device.Wait();
    swapchain.Resize(swapchain.extent, pendingVSync);
    std::vector<VkImage> rawImages = swapchain.GetImages();
    for (uint32_t i = 0; i < frameCount; i++)
        swapchainImages[i].WrapSwapchainImage(device.Get(), rawImages[i], swapchain.format, swapchain.extent);
    vsyncChangeRequested = false;
}

void Renderer::Render()
{
    if (renderPath) Render(*renderPath);
}

void Renderer::Render(RenderPath& path)
{
    if (!BeginFrame()) return;

    graph.BeginFrame(frameNumber, frameNumber - frameCount);
    graph.ImportImage("finalImage", &finalImage);
    graph.ImportImage("swapchain", &swapchainImages[imageIndex]);

    path.Build(graph);
    graph.Compile();
    graph.Execute(cmd);

    TransitionSet toPresent;
    toPresent.AddImage(&swapchainImages[imageIndex], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT);
    toPresent.Transition(cmd);

    EndFrame();
}

bool Renderer::BeginFrame()
{
    bool resized = false;
    if (windowResizeRequested) { WindowResize(); resized = true; }
    if (viewportResizeRequested) { ViewportResize(); resized = true; }
    if (vsyncChangeRequested) { ApplyVSync(); resized = true; }
    if (resized) return false;

    inFlightFences[currentFrame].Wait();
    inFlightFences[currentFrame].Reset();
    
    VkResult result = vkAcquireNextImageKHR(device.Get(), swapchain.Get(), UINT64_MAX, imageAvailableSemaphores[currentFrame].Get(), VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) { LOG_WARN("Swapchain out of date"); windowResizeRequested = true; return false; }
    
    commandBuffers[imageIndex].Reset();
    commandBuffers[imageIndex].Begin();
    cmd = commandBuffers[imageIndex].Get();

    return true;
}

void Renderer::EndFrame()
{
    commandBuffers[imageIndex].End();
    graphicsQueue.Submit(cmd, imageAvailableSemaphores[currentFrame].Get(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderFinishedSemaphores[currentFrame].Get(), inFlightFences[currentFrame].Get());
    presentQueue.Present(swapchain.Get(), imageIndex, renderFinishedSemaphores[currentFrame].Get());

    currentFrame = (currentFrame + 1) % frameCount;
    frameNumber++; 
}
