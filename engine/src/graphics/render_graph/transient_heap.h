#pragma once
#include "pch.h"
#include "transient_resource.h"

struct TransientRequest
{
    enum class Kind : uint8_t { Image, Buffer };
    Kind kind = Kind::Image;
    TransientImageDesc imageDesc;
    VkExtent2D extent = {};
    TransientBufferDesc bufferDesc;
    uint32_t firstPass = 0, lastPass = 0;
};

struct TransientHeap
{
    struct Slot {
        TransientRequest::Kind kind = TransientRequest::Kind::Image;
        uint32_t physIndex = 0;
        uint32_t bucket = 0;
        VkDeviceSize offset = 0;
        VkMemoryRequirements memReq = {};
        uint32_t firstPass = 0, lastPass = 0;
    };

    struct Bucket {
        TransientRequest::Kind kind = TransientRequest::Kind::Image;
        uint32_t memoryTypeBits = 0;
        VmaAllocation backing = VK_NULL_HANDLE;
        VkDeviceSize size = 0, maxAlign = 1;
        std::vector<uint32_t> members;    
    };

    struct RetiredImage { uint64_t frame; VkImage image; VkImageView view; };
    struct RetiredBuffer { uint64_t frame; VkBuffer buffer; };
    struct RetiredBacking { uint64_t frame; VmaAllocation backing; };

    VkDevice device = VK_NULL_HANDLE;
    VmaAllocator vma = VK_NULL_HANDLE;
    uint64_t currentHash = 0;
    bool realizedOnce = false;

    std::vector<PhysicalImage> images;
    std::vector<PhysicalBuffer> buffers;
    std::vector<Slot> slots;
    std::vector<Bucket> buckets;

    std::vector<RetiredImage> retiredImages;
    std::vector<RetiredBuffer> retiredBuffers;
    std::vector<RetiredBacking> retiredBackings;

    struct Stats {
        VkDeviceSize totalBackingBytes = 0;
        VkDeviceSize sumResourceBytes  = 0;
        uint32_t bucketCount = 0, imageCount = 0, bufferCount = 0;
    } stats;

    void Init(VkDevice device, VmaAllocator vma);
    void Shutdown();

    bool Realize(const std::vector<TransientRequest>& requests, uint64_t frameIndex);
    
    uint32_t FindOrCreateBucket(TransientRequest::Kind kind, uint32_t memoryTypeBits);
    VkDeviceSize PlaceBucket(Bucket& b);
    bool EnsureBacking(Bucket& b);
    void RetireCurrent(uint64_t frameIndex);
    void Recycle(uint64_t completedFrameIndex);
    
    void DumpStats() const;

    PhysicalImage&  GetImage (uint32_t slot);
    PhysicalBuffer& GetBuffer(uint32_t slot);

    const Stats& GetStats() const { return stats; }
};
