#include "Renderer.h"
#include "Core/Window.h"

bool Renderer::Create(Window& window)
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
    // hasAsyncCompute = physicalDevice.HasDedicatedCompute();
    
    // BE_ASSERT(graphicsQueue.Acquire(device.Get(), physicalDevice.graphicsFamily));
    // BE_ASSERT(presentQueue.Acquire(device.Get(), physicalDevice.presentFamily));
    // BE_ASSERT(transferQueue.Acquire(device.Get(), physicalDevice.transferFamily));
    // BE_ASSERT(computeQueue.Acquire(device.Get(), physicalDevice.computeFamily));

    // BE_ASSERT(swapchain.Create(physicalDevice.Get(), device.Get(), surface.Get(), { windowExtent.width, windowExtent.height }, settings.vsync));

    // VkAttachmentDescription swapchainAttachment{};
    // swapchainAttachment.format = swapchain.format;
    // swapchainAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // swapchainAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // swapchainAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // swapchainAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // swapchainAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // swapchainAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // swapchainAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // BE_ASSERT(swapchainRenderPass.Create(device.Get(), { swapchainAttachment }));

    // std::vector<VkImage> rawImages = swapchain.GetImages();
    // frameCount = static_cast<uint32_t>(rawImages.size());

    // swapchainImages.resize(frameCount);
    // swapchainFramebuffers.resize(frameCount);

    // for (uint32_t i = 0; i < frameCount; i++) {
    //     BE_ASSERT(swapchainImages[i].WrapSwapchainImage(device.Get(), rawImages[i], swapchain.format, swapchain.extent));
    //     BE_ASSERT(swapchainFramebuffers[i].Create(device.Get(), swapchainRenderPass.Get(), swapchain.extent, { swapchainImages[i].GetView() }));
    // }
    // return true;

    return true;
}

void Renderer::Destroy()
{

}

void Renderer::Render()
{
    
}
