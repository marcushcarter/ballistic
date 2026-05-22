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

bool Buffer::Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& props, VkDeviceSize initialCapacity, VkBufferUsageFlags usageFlags, bool isHostVisible)
{
    VK_CHECK_HANDLE(device, VkDevice);

    Destroy();
    size = 0;
    capacity = 0;
    usage = usageFlags;
    hostVisible = isHostVisible;
    deviceHandle = device;
    memoryProps = &props;

    if (!isHostVisible)
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = std::max<VkDeviceSize>(1, initialCapacity);
    createInfo.usage = usageFlags;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(device, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan buffer");
        return false;
    }

    VkMemoryRequirements memReq{};
    vkGetBufferMemoryRequirements(device, buffer, &memReq);

    capacity = memReq.size;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = FindMemoryType(props, memReq.memoryTypeBits, isHostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate Vulkan GPU memory");
        return false;
    }

    if (vkBindBufferMemory(device, buffer, memory, 0) != VK_SUCCESS) {
        LOG_ERROR("Failed to bind Vulkan buffer memory");
        return false;
    }

    if (isHostVisible)
        vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, &mappedPtr);
    
    LOG_DEBUG("Buffer created: (%d) bytes", capacity);
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
        LOG_DEBUG("Buffer destroyed");
    }
    size = 0;
    capacity = 0;
}

bool Buffer::Update(void* data, VkDeviceSize dataSize, VkDeviceSize offset)
{
    if (!hostVisible || !mappedPtr) return false;

    VkDeviceSize end = offset + dataSize;
    bool resized = false;

    if (end > capacity) {
        resized = true;

        VkDeviceSize newCapacity = NextPowerOfTwo(std::max(capacity * 2, end));

        VkBuffer newBuffer = VK_NULL_HANDLE;
        VkDeviceMemory newMemory = VK_NULL_HANDLE;
        
        VkBufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.size = newCapacity;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        if (vkCreateBuffer(deviceHandle, &createInfo, nullptr, &newBuffer) != VK_SUCCESS) {
            LOG_ERROR("Failed to create Vulkan buffer");
            return false;
        }

        VkMemoryRequirements memReq{};
        vkGetBufferMemoryRequirements(deviceHandle, newBuffer, &memReq);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = FindMemoryType(*memoryProps, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(deviceHandle, &allocInfo, nullptr, &newMemory) != VK_SUCCESS) {
            LOG_ERROR("Failed to allocate Vulkan GPU memory");
            return false;
        }

        if (vkBindBufferMemory(deviceHandle, newBuffer, newMemory, 0) != VK_SUCCESS) {
            LOG_ERROR("Failed to bind buffer memory");
            return false;
        }

        if (size > 0 && mappedPtr) {
            void* newMapped = nullptr;
            vkMapMemory(deviceHandle, newMemory, 0, allocInfo.allocationSize, 0, &newMapped);
            memcpy(newMapped, mappedPtr, size);
            vkUnmapMemory(deviceHandle, newMemory);
        }

        vkDestroyBuffer(deviceHandle, buffer, nullptr);
        vkFreeMemory(deviceHandle, memory, nullptr);

        buffer = newBuffer;
        memory = newMemory;

        vkMapMemory(deviceHandle, memory, 0, VK_WHOLE_SIZE, 0, &mappedPtr);
        LOG_DEBUG("Buffer resized from %d bytes to %d bytes", capacity, newCapacity);
        capacity = newCapacity;
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

    VkBuffer newBuffer = VK_NULL_HANDLE;
    VkDeviceMemory newMemory = VK_NULL_HANDLE;

    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = newCapacity;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(deviceHandle, &createInfo, nullptr, &newBuffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan buffer", false);
        return false;
    }

    VkMemoryRequirements memReq{};
    vkGetBufferMemoryRequirements(deviceHandle, newBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        *memoryProps,
        memReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    if (vkAllocateMemory(deviceHandle, &allocInfo, nullptr, &newMemory) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate Vulkan GPU memory");
        vkDestroyBuffer(deviceHandle, newBuffer, nullptr);
        return false;
    }

    if (vkBindBufferMemory(deviceHandle, newBuffer, newMemory, 0) != VK_SUCCESS) {
        LOG_ERROR("Failed to bind buffer memory");
        vkDestroyBuffer(deviceHandle, newBuffer, nullptr);
        vkFreeMemory(deviceHandle, newMemory, nullptr);
        return false;
    }

    if (mappedPtr && size > 0) {
        void* newMapped = nullptr;
        vkMapMemory(deviceHandle, newMemory, 0, allocInfo.allocationSize, 0, &newMapped);
        memcpy(newMapped, mappedPtr, size);
        vkUnmapMemory(deviceHandle, newMemory);
    }

    if (mappedPtr) vkUnmapMemory(deviceHandle, memory);
    vkDestroyBuffer(deviceHandle, buffer, nullptr);
    vkFreeMemory(deviceHandle, memory, nullptr);

    buffer = newBuffer;
    memory = newMemory;
    
    LOG_DEBUG("Buffer resized from %d bytes to %d bytes", capacity, newCapacity);
    capacity = newCapacity;
    vkMapMemory(deviceHandle, memory, 0, VK_WHOLE_SIZE, 0, &mappedPtr);
    
    return true;
}

bool Buffer::Copy(VkCommandBuffer cmd, VkBuffer srcBuffer, VkDeviceSize copySize, VkDeviceSize srcOffset, VkDeviceSize dstOffset)
{
    // VK_CHECK_HANDLE(cmd, VkCommandBuffer, false);
    // VK_CHECK_HANDLE(srcBuffer, VkBuffer, false);

    if (!(usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)) {
        LOG_WARN("dst buffer missing TRANSFER_DST_BIT");
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
