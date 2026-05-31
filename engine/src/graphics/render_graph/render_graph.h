#pragma once
#include "pch.h"
#include "pass.h"
#include "graphics/vk/image/image_2d.h"
#include "graphics/vk/buffer/buffer.h"
#include "transient_resource.h"
#include "transient_heap.h"

struct Image2D;
struct Pass;

struct ResourceHandle {
    uint32_t resource = UINT32_MAX;
    uint16_t version = 0;
    bool IsValid() const { return resource != UINT32_MAX; }
};

struct RenderGraph
{
    struct Use {
        ResourceHandle handle;
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 access = VK_ACCESS_2_NONE;
        bool isWrite = false;
    };

    struct Node {
        std::unique_ptr<Pass> pass;
        std::vector<Use> uses;
        VkQueueFlagBits queue = VK_QUEUE_GRAPHICS_BIT;
        uint32_t refCount = 0;
        bool active = true;
    };

    enum class Kind { ExternalImage, ExternalBuffer, TransientImage, TransientBuffer };

    struct Resource {
        Kind kind = Kind::TransientImage;
        const char* name = nullptr;

        Image2D* externalImage = nullptr;
        Buffer* externalBuffer = nullptr;
        TransientImageDesc imageDesc;
        TransientBufferDesc bufferDesc;
        VkExtent2D resolvedExtent = {};

        struct Version { int32_t producerPass = -1; uint32_t readRefCount = 0; };
        std::vector<Version> versions;

        int32_t firstPass = INT32_MAX;
        int32_t lastPass = -1;
        bool referenced = false;
        int32_t heapSlot = -1;
    };

    struct Barrier {
        ResourceHandle handle;
        VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkPipelineStageFlags2 srcStage = 0, dstStage = 0;
        VkAccessFlags2 srcAccess = 0, dstAccess = 0;
        bool isImage = true;
    };
    
    struct Step {
        Node* node = nullptr;
        std::vector<Barrier> barriers;
    };

    std::vector<Resource> resources;
    std::unordered_map<std::string, uint32_t> nameMap;
    std::vector<Node> nodes;
    std::vector<Step> plan;
    Node* currentSetup = nullptr;
    int32_t currentPassIdx = -1;

    TransientHeap heap;
    VkExtent2D viewportExtent = {};
    uint64_t frameIndex = 0;
    VkDevice device = VK_NULL_HANDLE;
    VmaAllocator vma = VK_NULL_HANDLE;

    void Init(VkDevice device, VmaAllocator vma);
    void Shutdown();
    void SetViewport(VkExtent2D extent);
    void BeginFrame(uint64_t frameIndex, uint64_t completedFrameIndex);
    void Reset();

    int32_t Resolve(const char* name);
    ResourceHandle RecordWrite(uint32_t resource, VkImageLayout layout, VkPipelineStageFlags2 stage, VkAccessFlags2 access);
    ResourceHandle RecordRead(uint32_t resource, VkImageLayout layout, VkPipelineStageFlags2 stage, VkAccessFlags2 access);

    ResourceHandle ImportImage(const char* name, Image2D* image);
    ResourceHandle ImportBuffer(const char* name, Buffer* buffer);
    ResourceHandle CreateImage(const char* name, const TransientImageDesc& desc, VkImageLayout layout, VkPipelineStageFlags2 stage, VkAccessFlags2 access);
    ResourceHandle CreateBuffer(const char* name, const TransientBufferDesc& desc, VkPipelineStageFlags2 stage, VkAccessFlags2 access);
    
    ResourceHandle ReadImage(const char* name, VkImageLayout layout, VkPipelineStageFlags2 stage, VkAccessFlags2 access);
    ResourceHandle WriteImage(const char* name, VkImageLayout layout, VkPipelineStageFlags2 stage, VkAccessFlags2 access);
    ResourceHandle ReadBuffer(const char* name, VkPipelineStageFlags2 stage, VkAccessFlags2 access);
    ResourceHandle WriteBuffer(const char* name, VkPipelineStageFlags2 stage, VkAccessFlags2 access);
    
    void AddPass(std::unique_ptr<Pass> pass);
    
    void Compile();
    void Execute(VkCommandBuffer cmd);

    VkImageView GetImageView(ResourceHandle h);
    VkImage GetVkImage(ResourceHandle h);
    VkExtent2D GetImageExtent(ResourceHandle h);
    VkImageAspectFlags GetImageAspect(ResourceHandle h);
    VkBuffer GetVkBuffer(ResourceHandle h);
};
