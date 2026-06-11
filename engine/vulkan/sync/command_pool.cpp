#include <sync/command_pool.h>
#include <misc/utils.h>

namespace Ballistic::Vulkan
{
bool CommandPool::Create(VkDevice device, const CommandPoolDesc& d)
{
    Destroy();
    debugName = d.debugName;
    deviceHandle = device;

    VkCommandPoolCreateFlags flags = 0;
    if (d.transient) flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (d.resetable) flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = d.queueFamilyIndex;
    createInfo.flags = flags;

    if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS) {
        // LOG_ERROR("Command Pool create failed: %s - vkCreateCommandPool", debugName ? debugName : "Unnamed");
        return false;
    }

    SetObjectName(device, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)commandPool, d.debugName);
    // LOG_DEBUG("Command Pool created: %s", debugName ? debugName : "Unnamed");
    return true;
}

void CommandPool::Destroy()
{
    if (commandPool) {
        vkDestroyCommandPool(deviceHandle, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
        // LOG_DEBUG("Command Pool destroyed: %s", debugName ? debugName : "Unnamed");
    }
}

void CommandPool::Reset(VkCommandPoolResetFlags flags) { vkResetCommandPool(deviceHandle, commandPool, flags); }
}