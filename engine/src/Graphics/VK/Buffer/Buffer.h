#pragma once
#include "pch.h"

// struct BufferDesc
// {
//     VkDeviceSize size = 0;
//     VkBufferUsageFlags usage = 0;
//     bool hostVisible = false;
//     const char* name = nullptr;

//     static BufferDesc Uniform(VkDeviceSize size, const char* name = nullptr) {
//         return { size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true, name };
//     }
    
//     static BufferDesc Storage(VkDeviceSize size, bool hostVisible = false, const char* name = nullptr, VkBufferUsageFlags extraUsage = 0) {
//         return { size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | extraUsage, hostVisible, name };
//     }
    
//     static BufferDesc Indirect(VkDeviceSize size, bool hostVisible = false, const char* name = nullptr, VkBufferUsageFlags extraUsage = 0) {
//         return { size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | extraUsage, hostVisible, name };
//     }
    
//     static BufferDesc Vertex(VkDeviceSize size, bool hostVisible = false, const char* name = nullptr, VkBufferUsageFlags extraUsage = 0) {
//         return { size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | extraUsage, hostVisible, name };
//     }
    
//     static BufferDesc Index(VkDeviceSize size, bool hostVisible = false, const char* name = nullptr, VkBufferUsageFlags extraUsage = 0) {
//         return { size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | extraUsage, hostVisible, name };
//     }
    
//     static BufferDesc Staging(VkDeviceSize size, VkBufferUsageFlags extraUsage = 0) {
//         return { size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | extraUsage, true, nullptr };
//     }
// };

struct Buffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize capacity = 0;
    VkDeviceSize size = 0;
    VkBufferUsageFlags usage = 0;
    void* mappedPtr = nullptr;
    bool hostVisible = false;

    VkPipelineStageFlags stage = 0;
    VkAccessFlags access = 0;
    
    VkDevice deviceHandle = VK_NULL_HANDLE;
    const VkPhysicalDeviceMemoryProperties* memoryProps = nullptr;

    bool Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& props, VkDeviceSize capacity, VkBufferUsageFlags usage, bool hostVisible = false);
    void Destroy();

    bool Update(void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    bool Resize(VkDeviceSize newSize);
    bool Copy(VkCommandBuffer cmd, VkBuffer srcBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);

    void Transition(VkCommandBuffer cmd, VkPipelineStageFlags stage, VkAccessFlags access);
    
    VkBuffer Get() const { return buffer; }
};
