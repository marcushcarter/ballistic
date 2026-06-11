#include <core/queue.h>

namespace Ballistic::Vulkan
{
bool Queue::Acquire(VkDevice device, uint32_t family)
{
    familyIndex = family;
    vkGetDeviceQueue(device, familyIndex, 0, &queue);
    return true;
}

VkResult Queue::Submit(VkCommandBuffer cmd, VkSemaphore waitSemaphore, VkPipelineStageFlags waitStage, VkSemaphore signalSemaphore, VkFence fence)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    if (waitSemaphore != VK_NULL_HANDLE) {
        submitInfo.waitSemaphoreCount = waitSemaphore ? 1 : 0;
        submitInfo.pWaitSemaphores = &waitSemaphore;
        submitInfo.pWaitDstStageMask = &waitStage;
    }
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    
    if (signalSemaphore != VK_NULL_HANDLE) {
        submitInfo.signalSemaphoreCount = signalSemaphore ? 1 : 0;
        submitInfo.pSignalSemaphores = &signalSemaphore;
    }

    return vkQueueSubmit(queue, 1, &submitInfo, fence);
}

VkResult Queue::Present(VkSwapchainKHR swapchain, uint32_t imageIndex, VkSemaphore waitSemaphore)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = waitSemaphore ? 1 : 0;
    presentInfo.pWaitSemaphores = &waitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    return vkQueuePresentKHR(queue, &presentInfo);
}

void Queue::WaitIdle() { vkQueueWaitIdle(queue); }
}