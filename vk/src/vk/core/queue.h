#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>

struct Queue
{
    VkQueue queue = VK_NULL_HANDLE;
    uint32_t familyIndex = 0;

    bool Acquire(VkDevice device, uint32_t family);

    void Submit(VkCommandBuffer cmd, VkSemaphore waitSemaphore = VK_NULL_HANDLE, VkPipelineStageFlags waitStage = 0, VkSemaphore signalSemaphore = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE);
    void Present(VkSwapchainKHR swapchain, uint32_t imageIndex, VkSemaphore waitSemaphore);
    
    void WaitIdle();

    VkQueue Get() const { return queue; }
};
