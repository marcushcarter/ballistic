#include "Fence.h"

bool Fence::Create(VkDevice device, bool signaled)
{
    VK_CHECK_HANDLE(device, VkDevice);

    Destroy();
    deviceHandle = device;

    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    if (vkCreateFence(device, &createInfo, nullptr, &fence) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan fence");
        return false;
    }

    LOG_DEBUG("Fence created");
    return true;
}

void Fence::Destroy()
{
    if (fence != VK_NULL_HANDLE) {
        vkDestroyFence(deviceHandle, fence, nullptr);
        fence = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Fence destroyed");
    }
}

void Fence::Wait(uint64_t timeout)
{
    if (fence != VK_NULL_HANDLE) vkWaitForFences(deviceHandle, 1, &fence, VK_TRUE, timeout);
}

void Fence::Reset()
{
    if (fence != VK_NULL_HANDLE) vkResetFences(deviceHandle, 1, &fence);
}
