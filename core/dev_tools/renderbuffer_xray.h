#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
#include <core/log/error.h>

namespace ballistic {

struct RenderGraph;
namespace drivers { struct RenderingDeviceDriverVulkan; }

struct RenderBufferXray
{
    bool open = true;
    uint64_t selected_name_id = 0;
    
    drivers::RenderingDeviceDriverVulkan* device_driver = nullptr;

    VkSampler sampler = VK_NULL_HANDLE;
    VkDescriptorSet set = VK_NULL_HANDLE;
    VkImageView built_for_view = VK_NULL_HANDLE;
    VkDescriptorSet pending_free = VK_NULL_HANDLE;
    uint64_t pending_free_frame = 0;

    Error init(drivers::RenderingDeviceDriverVulkan& dd);
    void shutdown();

    void draw(RenderGraph& graph, uint64_t frame, uint32_t frame_count);
};

}