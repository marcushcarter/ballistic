#pragma once
#include <core/log/error.h>
#include <drivers/vulkan/ballistic_vulkan.h>
#include <unordered_map>
#include <vector>

namespace ballistic {

namespace drivers { struct DeviceDriverVulkan; }

struct ImGuiTextureCache
{
    struct Entry {
        VkDescriptorSet set = VK_NULL_HANDLE;
        uint64_t last_used_frame = 0;
    };

    struct Retired {
        VkDescriptorSet set = VK_NULL_HANDLE;
        uint64_t retire_frame = 0;
    };

    drivers::DeviceDriverVulkan* device_driver = nullptr;
    VkSampler sampler = VK_NULL_HANDLE;

    std::unordered_map<VkImageView, Entry> entries;
    std::vector<Retired> retired;

    Error create(drivers::DeviceDriverVulkan& r_device_driver);
    void destroy();

    VkDescriptorSet get(VkImageView p_view);
    void retire(VkImageView p_view, uint64_t p_frame_number, uint32_t p_frame_count);
    void collect(uint64_t frame_number);

    void invalidate_all();
};

}