#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>

namespace Ballistic::Vulkan
{
struct Fence
{
    VkFence fence = VK_NULL_HANDLE;
    const char* debugName = nullptr;
    
    bool Create(VkDevice device, bool signaled = true, const char* debugName = nullptr);
    void Destroy();
    
    void Wait(uint64_t timeout = UINT64_MAX);
    void Reset();
    
    VkFence Get() const { return fence; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};
}