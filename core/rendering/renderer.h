#pragma once
#include <drivers/vulkan/device_driver_vulkan.h>
#include <core/rendering/render_graph.h>
#include <core/log/error.h>

namespace ballistic {

struct Renderer
{
    drivers::DeviceDriverVulkan* device_driver = nullptr;

    RenderGraph graph;

    uint32_t width = 0;
    uint32_t height = 0;
    uint64_t resize_epoch = 0;

    uint32_t frame_count = 1;
    uint32_t current_frame = 0;
    uint64_t frame_number = 0;

    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkFence> in_flight_fences;
    std::vector<drivers::DeviceDriverVulkan::CommandPool> command_pools;
    std::vector<VkCommandBuffer> command_buffers;
    VkCommandBuffer cmd = VK_NULL_HANDLE;

    drivers::DeviceDriverVulkan::Image final_image;
    drivers::DeviceDriverVulkan::Image depth_image;
    drivers::DeviceDriverVulkan::Image image_2;

    Error create(drivers::DeviceDriverVulkan& r_device_driver);
    void destroy();

    Error set_size(uint32_t p_width, uint32_t p_height);

    Error begin_frame();
    Error end_frame();
};

}