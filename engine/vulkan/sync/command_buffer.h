#pragma once
#include <vulkan/vulkan.h>

namespace Ballistic::Vulkan
{
struct CommandBuffer
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    const char* debugName = nullptr;
    
    bool Allocate(VkDevice device, VkCommandPool commandPool, bool secondary = false, const char* debugName = nullptr);
    void Free();

    bool Begin(VkCommandBufferUsageFlags flags = 0);
    void End();
    void Reset();
    
    VkCommandBuffer Get() const { return commandBuffer; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
    VkCommandPool commandPoolHandle = VK_NULL_HANDLE;
};
}