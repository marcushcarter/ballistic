#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>

struct DescriptorPoolDesc
{
    uint32_t samplers = 0;
    uint32_t combinedImageSamplers = 0;
    uint32_t sampledImages = 0;
    uint32_t storageImages = 0;
    uint32_t uniformTexelBuffers = 0;
    uint32_t storageTexelBuffers = 0;
    uint32_t uniformBuffers = 0;
    uint32_t storageBuffers = 0;
    uint32_t uniformBuffersDynamic = 0;
    uint32_t storageBuffersDynamic = 0;
    uint32_t inputAttachments = 0;

    bool freeable  = true;
    bool afterBind = true;
    const char* debugName = nullptr;
};

struct DescriptorPool
{
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    const char* debugName = nullptr;
    
    bool Create(VkDevice device, const DescriptorPoolDesc& desc);
    void Destroy();
    
    VkDescriptorPool Get() const { return descriptorPool; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};
