#include "Renderer.h"
#include "Core/Window.h"

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
    
    BE_ASSERT(commandPool.Create(device.Get(), graphicsQueue.familyIndex));
    
    commandBuffers.resize(frameCount);
    imageAvailableSemaphores.resize(frameCount);
    renderFinishedSemaphores.resize(frameCount);
    inFlightFences.resize(frameCount);

    for (uint32_t i = 0; i < frameCount; i++) {
        BE_ASSERT(commandBuffers[i].Allocate(device.Get(), commandPool.Get()));
        BE_ASSERT(imageAvailableSemaphores[i].Create(device.Get()));
        BE_ASSERT(renderFinishedSemaphores[i].Create(device.Get()));
        BE_ASSERT(inFlightFences[i].Create(device.Get()));
    }
    
    BE_ASSERT(allocator.Create(instance.Get(), physicalDevice.Get(), device.Get()));
    BE_ASSERT(descriptorPool.Create(device.Get()));

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

    BE_ASSERT(finalImageInputSetLayout.Create(device.Get(), { SetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) }));
    BE_ASSERT(finalImageInputSet.Allocate(device.Get(), descriptorPool.Get(), finalImageInputSetLayout.Get()));
    finalImageInputSet.SetImages(0, { finalImage.GetView() }, nearestSampler.Get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    BE_ASSERT(blitPipelineLayout.Create(device.Get(), { finalImageInputSetLayout.Get() }, {}));

//     static constexpr const char* fullscreen_vert_glsl = R"(
// #version 450
// layout(location = 0) out vec2 outUV;
// void main()
// {
//     outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
//     gl_Position = vec4(outUV * 2.0 - 1.0, 0.0, 1.0);
// }
// )";

//     static constexpr const char* blit_frag_glsl = R"(
// #version 450
// layout(location = 0) in vec2 inUV;
// layout(location = 0) out vec4 outColor;
// layout(set = 0, binding = 0) uniform sampler2D finalImage;
// void main()
// {
//     outColor = texture(finalImage, inUV);
// }
// )";

    // Shader vert{}, frag{};
    // // BE_ASSERT(vert.Compile(device.Get(), VK_SHADER_STAGE_VERTEX_BIT, CompileGLSL(fullscreen_vert_glsl, "fullscreen.vert")));
    // // BE_ASSERT(frag.Compile(device.Get(), VK_SHADER_STAGE_FRAGMENT_BIT, CompileGLSL(blit_frag_glsl, "blit.frag")));
    // BE_ASSERT(vert.CompileGLSL(device.Get(), VK_SHADER_STAGE_VERTEX_BIT,   fullscreen_vert_glsl, "fullscreen.vert"));
    // BE_ASSERT(frag.CompileGLSL(device.Get(), VK_SHADER_STAGE_FRAGMENT_BIT, blit_frag_glsl, "blit.frag"));

    // auto renderingInfo = PipelineRenderingInfo({ swapchain.format });
    // BE_ASSERT(blitPipeline.Create(device.Get(), blitPipelineLayout.Get(), VK_NULL_HANDLE, {
    //     .pNext = &renderingInfo,
    //     .shaderStages = { PipelineShaderStage(vert.Get(), vert.stage), PipelineShaderStage(frag.Get(), frag.stage) }
    // }));

    return true;
}

bool Renderer::CreateImGui(GLFWwindow* window)
{
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

    initInfo.DescriptorPool = descriptorPool.Get();
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
    commandPool.Destroy();

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
