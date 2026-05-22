#include "CommandPool.h"

bool CommandPool::Create(VkDevice device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
    VK_CHECK_HANDLE(device, VkDevice);

    Destroy();
    deviceHandle = device;
    
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = queueFamilyIndex;
    createInfo.flags = flags;

    if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan command pool");
        return false;
    }

    LOG_DEBUG("Command Pool created");
    return true;
}

void CommandPool::Destroy()
{
    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(deviceHandle, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Command Pool destroyed");
    }
}

void CommandPool::Reset(VkCommandPoolResetFlags flags)
{
    if (commandPool != VK_NULL_HANDLE) vkResetCommandPool(deviceHandle, commandPool, flags);
}
