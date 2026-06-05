#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>

struct CommandPoolDesc
{
    uint32_t queueFamilyIndex = 0;
    bool transient = false;
    bool resetable = false;
    const char* debugName = nullptr;
};

struct CommandPool
{
    VkCommandPool commandPool = VK_NULL_HANDLE;
    const char* debugName = nullptr;
    
    bool Create(VkDevice device, const CommandPoolDesc& desc);
    void Destroy();
    
    void Reset(VkCommandPoolResetFlags flags);
    
    VkCommandPool Get() const { return commandPool; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};