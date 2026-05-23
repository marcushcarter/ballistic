#include "CommandBuffer.h"

bool CommandBuffer::Allocate(VkDevice device, VkCommandPool commandPool, bool secondary, const char* name)
{
    VK_CHECK_HANDLE(device, VkDevice);
    VK_CHECK_HANDLE(commandPool, VkCommandPool);

    Free();
    debugName = name;
    deviceHandle = device;
    commandPoolHandle = commandPool;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        LOG_ERROR("Command Buffer allocate failed: %s - vkAllocateCommandBuffers", debugName ? debugName : "Unnamed");
        return false;
    }

    if (debugName) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
        nameInfo.objectHandle = (uint64_t)commandBuffer;
        nameInfo.pObjectName = debugName;
        auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
        if (func) func(device, &nameInfo);
    }

    LOG_DEBUG("Command Buffer allocated: %s", debugName ? debugName : "Unnamed");
    return true;
}

void CommandBuffer::Free()
{
    if (commandBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(deviceHandle, commandPoolHandle, 1, &commandBuffer);
        commandBuffer = VK_NULL_HANDLE;
        LOG_DEBUG("Command Buffer freed: %s", debugName ? debugName : "Unnamed");
    }
}

bool CommandBuffer::Begin(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flags;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        LOG_ERROR("Command Buffer begin failed: %s - could not begin recording", debugName ? debugName : "Unnamed");
        return false;
    }

    return true;
}

void CommandBuffer::End() { vkEndCommandBuffer(commandBuffer); }

void CommandBuffer::Reset() { vkResetCommandBuffer(commandBuffer, 0); }
