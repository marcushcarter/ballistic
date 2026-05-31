#pragma once
#include "pch.h"

struct BufferDesc
{
    VkDeviceSize size = 0;
    VkBufferUsageFlags usage = 0;
    bool hostVisible = false;
    const char* debugName = nullptr;

    static BufferDesc Vertex(VkDeviceSize size, bool hostVisible = false, const char* debugName = nullptr) {
        return { size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, hostVisible, debugName };
    }

    static BufferDesc Index(VkDeviceSize size, bool hostVisible = false, const char* debugName = nullptr) {
        return { size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, hostVisible, debugName };
    }

    static BufferDesc Uniform(VkDeviceSize size, const char* debugName = nullptr) {
        return { size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true, debugName };
    }

    static BufferDesc Storage(VkDeviceSize size, bool hostVisible = false, const char* debugName = nullptr) {
        return { size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, hostVisible, debugName };
    }

    static BufferDesc Indirect(VkDeviceSize size, bool hostVisible = false, const char* debugName = nullptr) {
        return { size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, hostVisible, debugName };
    }

    static BufferDesc Staging(VkDeviceSize size) {
        return { size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, nullptr };
    }
};

struct Buffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    const char* debugName = nullptr;

    VkDeviceSize capacity = 0;
    VkDeviceSize size = 0;
    VkBufferUsageFlags usage = 0;
    void* mappedPtr = nullptr;
    bool hostVisible = false;

    VkPipelineStageFlags stage = 0;
    VkAccessFlags access = 0;

    bool Create(VkDevice device, VmaAllocator vma, const BufferDesc& desc);
    void Destroy();

    bool Update(void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    bool Resize(VkDeviceSize newSize);
    bool Copy(VkCommandBuffer cmd, VkBuffer srcBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);
    void Transition(VkCommandBuffer cmd, VkPipelineStageFlags stage, VkAccessFlags access);

    void BindIndex(VkCommandBuffer cmd);
    void BindVertex(VkCommandBuffer cmd);
    
    VkBuffer Get() const { return buffer; }

private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
    VmaAllocator vmaHandle = VK_NULL_HANDLE;
};
