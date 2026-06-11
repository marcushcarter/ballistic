#pragma once
#include <buffer/buffer.h>
#include <vector>

namespace Ballistic::Vulkan
{
struct FrameRing
{
    std::vector<Buffer> buffers;

    bool Create(VkDevice device, VmaAllocator vma, uint32_t frameCount, const BufferDesc& desc);
    void Destroy();
    
    Buffer& Current(uint32_t frameIndex) { return buffers[frameIndex]; }
};
}