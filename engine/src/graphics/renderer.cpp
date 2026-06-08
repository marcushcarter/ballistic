#include <graphics/renderer.h>
#include <core/window.h>
#include <project/project.h>
#include <resources.h>
#include <graphics/render_graph/render_path.h>
#include <core/assert.h>
#include <core/log.h>
#include <vulkan/vulkan.hpp>
#include <stb_image.h>

inline bool LoadRCImage(VkDevice device, VmaAllocator vma, VkCommandBuffer cmd, int resourceID, Image2D& outImage, Buffer& outStaging, const char* debugName = nullptr)
{
    HRSRC res = FindResource(nullptr, MAKEINTRESOURCE(resourceID), RT_RCDATA);
    if (!res) {
        LOG_ERROR("LoadRCImage failed: resource %d not found", resourceID);
        return false;
    }
    
    HGLOBAL mem = LoadResource(nullptr, res);
    void* data = LockResource(mem);
    DWORD size = SizeofResource(nullptr, res);

    int w, h, channels;
    stbi_uc* pixels = stbi_load_from_memory((const stbi_uc*)data, (int)size, &w, &h, &channels, 4);
    if (!pixels) {
        LOG_ERROR("LoadRCImage failed: stbi decode failed for resource %d", resourceID);
        return false;
    }

    VkDeviceSize imageSize = (VkDeviceSize)w * h * 4;

    if (!outImage.Create(device, vma, {
        .extent = { (uint32_t)w, (uint32_t)h },
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
        .debugName = debugName
    })) { stbi_image_free(pixels); return false; }

    if (!outStaging.Create(device, vma, {
        .size = imageSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .hostVisible = true
    })) { stbi_image_free(pixels); return false; }

    outStaging.Update(pixels, imageSize);
    stbi_image_free(pixels);

    outImage.Transition(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
    outImage.CopyBuffer(cmd, outStaging.Get());
    outImage.Transition(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);

    return true;
}

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

    LOG_INFO("Vulkan %d.%d.%d - Driver %d.%d.%d - Using Device: %s (%s)",
        VK_API_VERSION_MAJOR(physicalDevice.properties.apiVersion),
        VK_API_VERSION_MINOR(physicalDevice.properties.apiVersion),
        VK_API_VERSION_PATCH(physicalDevice.properties.apiVersion),
        VK_API_VERSION_MAJOR(physicalDevice.properties.driverVersion),
        VK_API_VERSION_MINOR(physicalDevice.properties.driverVersion),
        VK_API_VERSION_PATCH(physicalDevice.properties.driverVersion),
        physicalDevice.properties.deviceName,
        vk::to_string(vk::PhysicalDeviceType(physicalDevice.properties.deviceType)).c_str()
    );

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

    BE_ASSERT(finalImage.Create(device.Get(), allocator.Get(), {
        .extent = swapchain.extent,
        .format = VK_FORMAT_R16G16B16A16_SFLOAT,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
        .debugName = "FinalImage"
    }));
    
    BE_ASSERT(linearSampler.Create(device.Get(), {
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .debugName  = "LinearSampler"
    }));

    BE_ASSERT(bindlessHeap.Create(device.Get(), {
        .sampledImages = 16384,
        .storageImages = 4096,
        .samplers = 256,
        .debugName = "BindlessHeap"
    }));

    BE_ASSERT(globalPipelineLayout.Create(device.Get(), {
        .setLayouts = { bindlessHeap.GetLayout() },
        .pushConstants = { PushConstant(VK_SHADER_STAGE_ALL, 0, 128) },
        .debugName = "GlobalPipelineLayout"
    }));

    {
        CommandPool transferCommandPool;
        BE_ASSERT(transferCommandPool.Create(device.Get(), {
            .queueFamilyIndex = transferQueue.familyIndex,
            .transient = true,
            .debugName = "TransferCommandPool"
        }));
        
        CommandBuffer transferCmd;
        BE_ASSERT(transferCmd.Allocate(device.Get(), transferCommandPool.Get(), false, "SplashTransferCommandBuffer"));
        transferCmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        std::vector<Buffer> stagingBuffers;
        
        Buffer& logoStaging = stagingBuffers.emplace_back();
        BE_ASSERT(LoadRCImage(device.Get(), allocator.Get(), transferCmd.Get(), IMG_LOGO_PNG, logoImage, logoStaging, "LogoImage"));

        Buffer& logoLongStaging = stagingBuffers.emplace_back();
        BE_ASSERT(LoadRCImage(device.Get(), allocator.Get(), transferCmd.Get(), IMG_LOGO_LONG_PNG, logoLongImage, logoLongStaging, "LogoLongImage"));
        
        transferCmd.End();
        transferQueue.Submit(transferCmd.Get());
        transferQueue.WaitIdle();

        for (auto& s : stagingBuffers) s.Destroy();
        transferCmd.Free();
        transferCommandPool.Destroy();
    }

    finalImage.bindlessSampled = bindlessHeap.RegisterSampledImage(finalImage.GetView());
    finalImage.bindlessStorage = bindlessHeap.RegisterStorageImage(finalImage.GetView());
    linearSampler.bindlessSampler = bindlessHeap.RegisterSampler(linearSampler.Get());

    graph.Init(device.Get(), allocator.Get(), &bindlessHeap);
    frameNumber = frameCount;

    graph.SetViewport(finalImage.extent);

    frameUniformRing.Create(device.Get(), allocator.Get(), frameCount, {
        BufferDesc::Uniform(128, "frameUniformBuffer")
    });

    LOG_DEBUG("Renderer started");
    return true;
}

void Renderer::Shutdown()
{
    device.Wait();

    UnloadProject();

    frameUniformRing.Destroy();

    graph.Shutdown();

    globalPipelineLayout.Destroy();
    bindlessHeap.Destroy();

    linearSampler.Destroy();
    logoImage.Destroy();
    logoLongImage.Destroy();
    finalImage.Destroy();

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

bool Renderer::LoadProject(const std::filesystem::path& path)
{
    auto start = std::chrono::high_resolution_clock::now();

    projectPath = path;
    BE_ASSERT(pipelineCache.Load(device.Get(), path / ".ballistic/cache/pipelines/pipeline_cache.bin"));

    if (renderPath) BE_ASSERT(renderPath->CreateResources(*this));

    BE_ASSERT(pipelineCache.Save(device.Get(), path / ".ballistic/cache/pipelines/pipeline_cache.bin"));
    pipelineCache.Destroy();

    auto end = std::chrono::high_resolution_clock::now();
    LOG_INFO("Pipeline creation took %lld ms", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    return true;
}

void Renderer::UnloadProject()
{
    device.Wait();
    if (renderPath) renderPath->DestroyResources();
    pipelineCache.Destroy();
    projectPath.clear();
    LOG_INFO("PROJECT CLOSED NOW");
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
    bindlessHeap.FreeSampledImage(finalImage.bindlessSampled);
    bindlessHeap.FreeStorageImage(finalImage.bindlessStorage);
    finalImage.bindlessSampled = bindlessHeap.RegisterSampledImage(finalImage.GetView());
    finalImage.bindlessStorage = bindlessHeap.RegisterStorageImage(finalImage.GetView());
        
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
    if (!renderPath || !BeginFrame()) return;
    
    struct TEMPBUFFERSTRUCT {
        float color[3] = { 1.0f, 0.5f, 1.0f };
    } temp;

    frameUniformRing.Current(currentFrame).Update(&temp, sizeof(TEMPBUFFERSTRUCT), 0);

    graph.BeginFrame(frameNumber, frameNumber - frameCount);
    
    FrameGraph fg{};
    fg.finalImage = graph.ImportImage("FinalImage", &finalImage);
    fg.swapchain = graph.ImportImage("Swapchain", &swapchainImages[imageIndex]);
    fg.frameUniform = graph.ImportBuffer("FrameUniformBuffer", &frameUniformRing.Current(currentFrame));
    
    VkDescriptorSet pDescriptorSets = { bindlessHeap.GetSet() };
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, globalPipelineLayout.Get(), 0, 1, &pDescriptorSets, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, globalPipelineLayout.Get(), 0, 1, &pDescriptorSets, 0, nullptr);

    renderPath->Build(graph, fg);
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
    if (result == VK_ERROR_OUT_OF_DATE_KHR) { LOG_WARN("Swapchain out of date"); windowResizeRequested = true;  return false; }
    
    commandBuffers[imageIndex].Reset();
    commandBuffers[imageIndex].Begin();
    cmd = commandBuffers[imageIndex].Get();

    Image2D& sc = swapchainImages[imageIndex];
    sc.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    sc.stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    sc.access = 0;

    return true;
}

void Renderer::EndFrame()
{
    commandBuffers[imageIndex].End();

    VkResult sub = graphicsQueue.Submit(cmd, imageAvailableSemaphores[currentFrame].Get(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderFinishedSemaphores[currentFrame].Get(), inFlightFences[currentFrame].Get());
    if (sub != VK_SUCCESS) LOG_ERROR("vkQueueSubmit: %d", sub);   // -4 == VK_ERROR_DEVICE_LOST

    VkResult pres = presentQueue.Present(swapchain.Get(), imageIndex, renderFinishedSemaphores[currentFrame].Get());
    if (pres != VK_SUCCESS && pres != VK_SUBOPTIMAL_KHR) LOG_ERROR("vkQueuePresent: %d", pres);

    currentFrame = (currentFrame + 1) % frameCount;
    frameNumber++; 
}
