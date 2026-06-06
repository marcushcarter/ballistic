#include <vk/buffer/frame_ring.h>

bool FrameRing::Create(VkDevice device, VmaAllocator vma, uint32_t frameCount, const BufferDesc& d)
{
    Destroy();
    buffers.resize(frameCount);

    for (uint32_t i = 0; i < frameCount; i++) {
        if (!buffers[i].Create(device, vma, d))
            return false;
    }
    return true;
}

void FrameRing::Destroy()
{
    for (Buffer& b : buffers) b.Destroy();
    buffers.clear();
}
