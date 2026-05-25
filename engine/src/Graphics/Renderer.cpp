#include "Renderer.h"
#include "Core/Window.h"
#include "Shaders.h"
#include "Resources.h"

inline bool LoadRCImage(VkDevice device, const VkPhysicalDeviceMemoryProperties& props, VkCommandBuffer cmd, int resourceID, Image2D& outImage, Buffer& outStaging, const char* debugName = nullptr)
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

    if (!outImage.Create(device, props, {
        .extent = { (uint32_t)w, (uint32_t)h },
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
        .debugName = debugName
    })) { stbi_image_free(pixels); return false; }

    if (!outStaging.Create(device, props, {
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
    
    BE_ASSERT(linearSampler.Create(device.Get(), {
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .debugName  = "LinearSampler"
    }));

    BE_ASSERT(finalImageInputSetLayout.Create(device.Get(), {
        .bindings = { SetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) },
        .debugName = "FinalImageInputSetLayout"
    }));
    
    BE_ASSERT(finalImageInputSet.Allocate(device.Get(), {
        .pool = descriptorPool.Get(),
        .setLayout = finalImageInputSetLayout.Get(),
        .debugName = "FinalImageInputSet"
    }));
    finalImageInputSet.SetImages(0, { finalImage.GetView() }, linearSampler.Get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    BE_ASSERT(blitPipelineLayout.Create(device.Get(), {
        .setLayouts = { finalImageInputSetLayout.Get() },
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

    CommandPool transferCommandPool;
    BE_ASSERT(transferCommandPool.Create(device.Get(), {
        .queueFamilyIndex = transferQueue.familyIndex,
        .transient = true,
        .debugName = "TransferCommandPool"
    }));
    
    CommandBuffer transferCmd;
    BE_ASSERT(transferCmd.Allocate(device.Get(), transferCommandPool.Get(), false, "TransferCommandBuffer"));
    transferCmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    std::vector<Buffer> stagingBuffers;
    
    Buffer& logoStaging = stagingBuffers.emplace_back();
    BE_ASSERT(LoadRCImage(device.Get(), physicalDevice.memory, transferCmd.Get(),
        IMG_LOGO_PNG, logoImage, logoStaging, "LogoImage"));

    Buffer& logoLongStaging = stagingBuffers.emplace_back();
    BE_ASSERT(LoadRCImage(device.Get(), physicalDevice.memory, transferCmd.Get(),
        IMG_LOGO_LONG_PNG, logoLongImage, logoLongStaging, "LogoLongImage"));
    
    transferCmd.End();
    transferQueue.Submit(transferCmd.Get());
    transferQueue.WaitIdle();

    for (auto& s : stagingBuffers) s.Destroy();
    transferCmd.Free();
    transferCommandPool.Destroy();

    LOG_DEBUG("Renderer started");
    return true;
}

bool Renderer::CreateImGui(GLFWwindow* window)
{
    BE_ASSERT(imguiDescriptorPool.Create(device.Get(), {
        .samplers = 10,
        .combinedImageSamplers = 10,
        .sampledImages = 10,        
        .afterBind = false,
        .debugName = "ImGuiDescriptorPool"
    }));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();

    VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
    pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingInfo.colorAttachmentCount = 1;
    pipelineRenderingInfo.pColorAttachmentFormats = &swapchain.format;

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = instance.Get();
    initInfo.PhysicalDevice = physicalDevice.Get();
    initInfo.Device = device.Get();
    initInfo.QueueFamily = graphicsQueue.familyIndex;
    initInfo.Queue = graphicsQueue.Get();
    
    // initInfo.PipelineCache = VK_NULL_HANDLE;

    initInfo.DescriptorPool = imguiDescriptorPool.Get();
    initInfo.MinImageCount = frameCount;
    initInfo.ImageCount = frameCount;
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingInfo;
    initInfo.CheckVkResultFn = [](VkResult err){ if(err) LOG_ERROR(" "); };

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_Init(&initInfo);

    HRSRC jbRes = FindResource(nullptr, MAKEINTRESOURCE(FONT_JETBRAINS_MONO_REGULAR_TTF), RT_RCDATA);
    HGLOBAL jbMem = LoadResource(nullptr, jbRes);
    DWORD jbSize = SizeofResource(nullptr, jbRes);

    void* jbData = IM_ALLOC(jbSize);
    memcpy(jbData, LockResource(jbMem), jbSize);

    ImFontConfig jbCfg;
    jbCfg.FontDataOwnedByAtlas = true;
    io.Fonts->AddFontFromMemoryTTF(jbData, (int)jbSize, 14.0f, &jbCfg);

    HRSRC faRes = FindResource(nullptr, MAKEINTRESOURCE(FONT_FA_SOLID_900_OTF), RT_RCDATA);
    HGLOBAL faMem = LoadResource(nullptr, faRes);
    DWORD faSize = SizeofResource(nullptr, faRes);

    void* faData = IM_ALLOC(faSize);
    memcpy(faData, LockResource(faMem), faSize);

    static const ImWchar faRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig faCfg;
    faCfg.MergeMode = true;
    faCfg.PixelSnapH = true;
    faCfg.FontDataOwnedByAtlas = true;
    io.Fonts->AddFontFromMemoryTTF(faData, (int)faSize, 14.0f, &faCfg, faRanges);
    io.Fonts->Build();

    LOG_DEBUG("ImGui created");
    return true;
}

void Renderer::Shutdown()
{
    device.Wait();

    logoImage.Destroy();
    logoLongImage.Destroy();

    blitPipeline.Destroy();
    blitPipelineLayout.Destroy();
    finalImageInputSetLayout.Destroy();
    linearSampler.Destroy();
    finalImage.Destroy();

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

void Renderer::DestroyImGui()
{
    device.Wait();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    imguiDescriptorPool.Destroy();
    LOG_DEBUG("ImGui destroyed");
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
    pendingSceneW = w;
    pendingSceneH = h;
    sceneResizeRequested = true;
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

void Renderer::SceneResize()
{
    device.Wait();
    finalImage.Resize({ pendingSceneW, pendingSceneH });
    finalImageInputSet.SetImages(0, { finalImage.GetView() }, linearSampler.Get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    sceneResizeRequested = false;
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

bool Renderer::BeginFrame()
{
    bool resized = false;
    if (windowResizeRequested) { WindowResize(); resized = true; }
    if (sceneResizeRequested) { SceneResize(); resized = true; }
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
    // --- Final Image Purplification ---

    TransitionSet toClear;
    toClear.AddImage(&finalImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    toClear.Transition(cmd);

    VkRenderingAttachmentInfo finalAttachment{};
    finalAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    finalAttachment.imageView = finalImage.GetView();
    finalAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    finalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    finalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    finalAttachment.clearValue.color = { 1.0f, 0.0f, 1.0f, 1.0f };

    VkRenderingInfo finalRendering{};
    finalRendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    finalRendering.renderArea = { {0, 0}, finalImage.extent };
    finalRendering.layerCount = 1;
    finalRendering.colorAttachmentCount = 1;
    finalRendering.pColorAttachments = &finalAttachment;

    vkCmdBeginRendering(cmd, &finalRendering);
    vkCmdEndRendering(cmd);

    // --- Swapchain Pass ---

    TransitionSet toAttachment;
    toAttachment.AddImage(&swapchainImages[imageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
    toAttachment.AddImage(&finalImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
    toAttachment.Transition(cmd);

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = swapchainImages[imageIndex].GetView();
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = { {0, 0}, swapchain.extent };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRendering(cmd, &renderingInfo);
    if (onSwapchainPass) onSwapchainPass(cmd);
    vkCmdEndRendering(cmd);

    TransitionSet toPresent;
    toPresent.AddImage(&swapchainImages[imageIndex], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT);
    toPresent.Transition(cmd);

    commandBuffers[imageIndex].End();
    graphicsQueue.Submit(cmd, imageAvailableSemaphores[currentFrame].Get(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderFinishedSemaphores[currentFrame].Get(), inFlightFences[currentFrame].Get());
    presentQueue.Present(swapchain.Get(), imageIndex, renderFinishedSemaphores[currentFrame].Get());

    currentFrame = (currentFrame + 1) % frameCount;   
}

bool Renderer::RenderLoadingScreen()
{
    bool resized = false;
    if (windowResizeRequested) { WindowResize(); resized = true; }
    if (sceneResizeRequested) { SceneResize(); resized = true; }
    if (vsyncChangeRequested) { ApplyVSync(); resized = true; }
    if (resized) return false;

    inFlightFences[currentFrame].Wait();
    inFlightFences[currentFrame].Reset();
    
    VkResult result = vkAcquireNextImageKHR(device.Get(), swapchain.Get(), UINT64_MAX, imageAvailableSemaphores[currentFrame].Get(), VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) { LOG_WARN("Swapchain out of date"); windowResizeRequested = true; return false; }
    
    commandBuffers[imageIndex].Reset();
    commandBuffers[imageIndex].Begin();
    cmd = commandBuffers[imageIndex].Get();

    // --- Final Image Purplification ---

    TransitionSet toClear;
    toClear.AddImage(&finalImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    toClear.Transition(cmd);

    VkRenderingAttachmentInfo finalAttachment{};
    finalAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    finalAttachment.imageView = finalImage.GetView();
    finalAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    finalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    finalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    finalAttachment.clearValue.color = { 1.0f, 0.0f, 1.0f, 1.0f };

    VkRenderingInfo finalRendering{};
    finalRendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    finalRendering.renderArea = { {0, 0}, finalImage.extent };
    finalRendering.layerCount = 1;
    finalRendering.colorAttachmentCount = 1;
    finalRendering.pColorAttachments = &finalAttachment;

    vkCmdBeginRendering(cmd, &finalRendering);
    vkCmdEndRendering(cmd);

    // --- Swapchain Pass ---

    TransitionSet toAttachment;
    toAttachment.AddImage(&swapchainImages[imageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
    toAttachment.AddImage(&finalImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
    toAttachment.Transition(cmd);

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = swapchainImages[imageIndex].GetView();
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = { {0, 0}, swapchain.extent };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRendering(cmd, &renderingInfo);
    // if (onSwapchainPass) onSwapchainPass(cmd);
    
    VKViewportScissor(cmd, 0, 0, static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height));
    blitPipeline.Bind(cmd);
    blitPipeline.DescriptorSets(cmd, { finalImageInputSet.Get() });
    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRendering(cmd);

    TransitionSet toPresent;
    toPresent.AddImage(&swapchainImages[imageIndex], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT);
    toPresent.Transition(cmd);

    commandBuffers[imageIndex].End();
    graphicsQueue.Submit(cmd, imageAvailableSemaphores[currentFrame].Get(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderFinishedSemaphores[currentFrame].Get(), inFlightFences[currentFrame].Get());
    presentQueue.Present(swapchain.Get(), imageIndex, renderFinishedSemaphores[currentFrame].Get());

    currentFrame = (currentFrame + 1) % frameCount;
    
    return true;
}