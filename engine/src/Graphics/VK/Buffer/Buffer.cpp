#include "Buffer.h"

inline uint32_t FindMemoryType(const VkPhysicalDeviceMemoryProperties& props, uint32_t typeFilter, VkMemoryPropertyFlags requiredFlags)
{
    for (uint32_t i = 0; i < props.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (props.memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}

inline VkDeviceSize NextPowerOfTwo(VkDeviceSize x)
{
    if (x == 0) return 1;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    if constexpr(sizeof(VkDeviceSize) == 8) x |= x >> 32;
    return x + 1;
}

bool Buffer::Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& props, const BufferDesc& desc)
{
    VK_CHECK_HANDLE(device, VkDevice);

    Destroy();
    size = 0;
    capacity = 0;
    usage = desc.usage;
    hostVisible = desc.hostVisible;
    debugName = desc.debugName;
    deviceHandle = device;
    memoryProps = &props;

    if (!hostVisible)
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = std::max<VkDeviceSize>(1, desc.size);
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(device, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
        LOG_ERROR("Buffer create failed: %s - vkCreateBuffer", debugName ? debugName : "Unnamed");
        return false;
    }

    VkMemoryRequirements memReq{};
    vkGetBufferMemoryRequirements(device, buffer, &memReq);

    capacity = memReq.size;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = FindMemoryType(props, memReq.memoryTypeBits, hostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        LOG_ERROR("Buffer create failed: %s - vkAllocateMemory", debugName ? debugName : "Unnamed");
        return false;
    }

    if (vkBindBufferMemory(device, buffer, memory, 0) != VK_SUCCESS) {
        LOG_ERROR("Buffer create failed: %s - vkBindBufferMemory", debugName ? debugName : "Unnamed");
        return false;
    }

    if (hostVisible)
        vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, &mappedPtr);

    if (debugName) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        nameInfo.objectHandle = (uint64_t)buffer;
        nameInfo.pObjectName = debugName;
        auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
        if (func) func(device, &nameInfo);
    }

    LOG_DEBUG("Buffer created: %s (%d bytes, usage %s, %s)",
        debugName ? debugName : "Unnamed",
        capacity,
        vk::to_string(vk::BufferUsageFlags(usage)).c_str(),
        hostVisible ? "Host Visible" : "Device Local"
    );

    return true;
}

void Buffer::Destroy()
{
    if (mappedPtr) {
        vkUnmapMemory(deviceHandle, memory);
        mappedPtr = nullptr;
    }
    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(deviceHandle, buffer, nullptr);
        vkFreeMemory(deviceHandle, memory, nullptr);
        buffer = VK_NULL_HANDLE;
        LOG_DEBUG("Buffer destroyed: %s", debugName ? debugName : "Unnamed");
    }
    size = 0;
    capacity = 0;
}

bool Buffer::Update(void* data, VkDeviceSize dataSize, VkDeviceSize offset)
{
    if (!hostVisible || !mappedPtr) {
        LOG_WARN("Buffer update failed: %s - not host visible", debugName ? debugName : "Unnamed");
        return false;
    }

    VkDeviceSize end = offset + dataSize;
    bool resized = false;

    if (end > capacity) {
        resized = true;
        if (!Resize(end)) return false;
    }

    memcpy((uint8_t*)mappedPtr + offset, data, dataSize);
    size = std::max(size, end);
    return resized;
}

bool Buffer::Resize(VkDeviceSize newSize)
{
    if (!hostVisible || newSize <= 0) return false;

    VkDeviceSize newCapacity = NextPowerOfTwo(newSize);
    if (newCapacity <= capacity) return false;

    std::vector<uint8_t> existing(size);
    if (size > 0 && mappedPtr)
        memcpy(existing.data(), mappedPtr, size);

    VkDeviceSize oldSize = size;
        
    if (!Create(deviceHandle, *memoryProps, {
        .size = newCapacity,
        .usage = usage,
        .hostVisible = hostVisible,
        .debugName = debugName
    })) return false;

    if (oldSize > 0)
        memcpy(mappedPtr, existing.data(), oldSize);
    
    size = oldSize;
    return true;
}

bool Buffer::Copy(VkCommandBuffer cmd, VkBuffer srcBuffer, VkDeviceSize copySize, VkDeviceSize srcOffset, VkDeviceSize dstOffset)
{
    VK_CHECK_HANDLE(cmd, VkCommandBuffer);
    VK_CHECK_HANDLE(srcBuffer, VkBuffer);

    if (!(usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)) {
        LOG_WARN("Buffer create failed: %s - missing TRANSFER_DST_BIT", debugName ? debugName : "Unnamed");
        return false;
    }

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = copySize;

    vkCmdCopyBuffer(cmd, srcBuffer, buffer, 1, &copyRegion);
    return true;
}

void Buffer::Transition(VkCommandBuffer cmd, VkPipelineStageFlags dstStage, VkAccessFlags dstAccess)
{
    VkPipelineStageFlags srcStage = stage ? stage : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkAccessFlags srcAccess = access;
    
    VkBufferMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = buffer;
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;
    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 1, &barrier, 0, nullptr);

    stage = dstStage;
    access = dstAccess;
}

void Buffer::BindIndex(VkCommandBuffer cmd)
{
    vkCmdBindIndexBuffer(cmd, buffer, 0, VK_INDEX_TYPE_UINT32);
}

void Buffer::BindVertex(VkCommandBuffer cmd)
{
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, offsets);
}
