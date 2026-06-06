#pragma once
#include <vk/buffer/buffer.h>
#include <vector>

struct FrameRing
{
    std::vector<Buffer> buffers;

    bool Create(VkDevice device, VmaAllocator vma, uint32_t frameCount, const BufferDesc& desc);
    void Destroy();
    
    Buffer& Current(uint32_t frameIndex) { return buffers[frameIndex]; }
};
