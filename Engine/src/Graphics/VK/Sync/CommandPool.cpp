#include "CommandPool.h"

bool CommandPool::Create(VkDevice device, const CommandPoolDesc& desc)
{
    VK_CHECK_HANDLE(device, VkDevice);

    Destroy();
    debugName = desc.debugName;
    deviceHandle = device;

    VkCommandPoolCreateFlags flags = 0;
    if (desc.transient) flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (desc.resetable) flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = desc.queueFamilyIndex;
    createInfo.flags = flags;

    if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS) {
        LOG_ERROR("Command Pool create failed: %s - vkCreateCommandPool", debugName ? debugName : "Unnamed");
        return false;
    }

    if (debugName) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_COMMAND_POOL;
        nameInfo.objectHandle = (uint64_t)commandPool;
        nameInfo.pObjectName = debugName;
        auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
        if (func) func(device, &nameInfo);
    }

    LOG_DEBUG("Command Pool created: %s", debugName ? debugName : "Unnamed");
    return true;
}

void CommandPool::Destroy()
{
    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(deviceHandle, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
        LOG_DEBUG("Command Pool destroyed: %s", debugName ? debugName : "Unnamed");
    }
}

void CommandPool::Reset(VkCommandPoolResetFlags flags)
{
    if (commandPool != VK_NULL_HANDLE) vkResetCommandPool(deviceHandle, commandPool, flags);
}
