#include "CommandBuffer.h"

bool CommandBuffer::Allocate(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level)
{
    // VK_CHECK_HANDLE(device, VkDevice, false);
    // VK_CHECK_HANDLE(commandPool, VkCommandPool, false);

    Free();
    deviceHandle = device;
    commandPoolHandle = commandPool;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan command buffer");
        return false;
    }

    LOG_DEBUG("Command Buffer allocated");
    return true;
}

void CommandBuffer::Free()
{
    if (commandBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(deviceHandle, commandPoolHandle, 1, &commandBuffer);
        commandBuffer = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        commandPoolHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Command Buffer freed");
    }
}

bool CommandBuffer::Begin(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flags;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        LOG_ERROR("Failed to begin Vulkan command buffer recording");
        return false;
    }

    return true;
}

void CommandBuffer::End() { vkEndCommandBuffer(commandBuffer); }

void CommandBuffer::Reset() { vkResetCommandBuffer(commandBuffer, 0); }
