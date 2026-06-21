#pragma once
#include <drivers/vulkan/rendering_device_driver_vulkan.h>
#include <core/error/error.h>
// #include <vector>

namespace ballistic {

struct Renderer
{
    drivers::RenderingDeviceDriverVulkan* device_driver = nullptr;

    drivers::RenderingDeviceDriverVulkan::Swapchain swapchain;

    uint32_t frame_count = 1;
    uint32_t current_frame = 0;

    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkFence> in_flight_fences;
    std::vector<drivers::RenderingDeviceDriverVulkan::CommandPool> command_pools;
    std::vector<VkCommandBuffer> command_buffers;
    VkCommandBuffer cmd = VK_NULL_HANDLE;

    Error create(uint32_t p_frame_count);
    void destroy();

    Error begin_frame();
    Error end_frame();
};

}