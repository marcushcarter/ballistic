#include "Renderer.h"
#include "Core/Window.h"
#include "Shaders/Shaders.h"

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

    BE_ASSERT(nearestSampler.Create(device.Get(), {
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .debugName = "NearestSampler"
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
    finalImageInputSet.SetImages(0, { finalImage.GetView() }, nearestSampler.Get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    BE_ASSERT(blitPipelineLayout.Create(device.Get(), {
        .setLayouts = { finalImageInputSetLayout.Get() },
        .debugName = "BlitPipelineLayout"
    }));

    Shader vert{}, frag{};
    BE_ASSERT(vert.Compile(device.Get(), VK_SHADER_STAGE_VERTEX_BIT, { FULLSCREEN_VERT_SPV, FULLSCREEN_VERT_SPV + FULLSCREEN_VERT_SPV_SIZE / 4 }));    
    BE_ASSERT(frag.Compile(device.Get(), VK_SHADER_STAGE_FRAGMENT_BIT, { BLIT_FRAG_SPV, BLIT_FRAG_SPV + BLIT_FRAG_SPV_SIZE / 4 }));

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

    // Buffer buffer{};
    // BE_ASSERT(buffer.Create(device.Get(), physicalDevice.memory, {
    //     .size = sizeof(float) * 4,
    //     .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    //     .hostVisible = false,
    //     .debugName = "Fullscreen Quad VB"
    // }));
    // buffer.Resize(sizeof(float) * 100);
    // float chars[9] = {
    //     -1.0f, -1.0f, 0.0f, 1.0f,
    //      3.0f, -1.0f, 0.0f, 1.0f,
    //     -1.0f
    // };
    // buffer.Update(chars, sizeof(chars));
    // buffer.Destroy();

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
    return true;
}

void Renderer::Shutdown()
{
    device.Wait();

    blitPipeline.Destroy();
    blitPipelineLayout.Destroy();
    finalImageInputSetLayout.Destroy();
    nearestSampler.Destroy();
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
}

void Renderer::DestroyImGui()
{
    device.Wait();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    imguiDescriptorPool.Destroy();
}

bool Renderer::BeginFrame()
{
    // if (resizeRequested) { Resize(); return false; }

    inFlightFences[currentFrame].Wait();
    inFlightFences[currentFrame].Reset();
    
    vkAcquireNextImageKHR(device.Get(), swapchain.Get(), UINT64_MAX, imageAvailableSemaphores[currentFrame].Get(), VK_NULL_HANDLE, &imageIndex);
    
    // VkResult result = vkAcquireNextImageKHR(device.Get(), swapchain.Get(), UINT64_MAX, imageAvailableSemaphores[currentFrame].Get(), VK_NULL_HANDLE, &imageIndex);
    // if (result == VK_ERROR_OUT_OF_DATE_KHR) { resizeRequested = true; return false; }
    
    commandBuffers[imageIndex].Reset();
    commandBuffers[imageIndex].Begin();
    cmd = commandBuffers[imageIndex].Get();

    return true;
}

void Renderer::EndFrame()
{
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
    finalRendering.renderArea = { {0, 0}, swapchain.extent };
    finalRendering.layerCount = 1;
    finalRendering.colorAttachmentCount = 1;
    finalRendering.pColorAttachments = &finalAttachment;

    vkCmdBeginRendering(cmd, &finalRendering);
    vkCmdEndRendering(cmd);













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
