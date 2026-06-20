#pragma once

#include <drivers/vulkan/rendering_context_driver_vulkan.h>

#include <shaderc/shaderc.hpp>
#include <vk_mem_alloc.h>

#include <drivers/vulkan/ballistic_vulkan.h>

namespace ballistic::drivers {

struct RenderingDeviceDriverVulkan
{
    // capabilities

    VkDevice vk_device = VK_NULL_HANDLE;
    RenderingContextDriverVulkan* context_driver = nullptr;
    DriverDevice context_device;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    uint32_t frame_count = 1;
    
    // 	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    // 	VkPhysicalDeviceProperties physical_device_properties = {};
    // 	VkPhysicalDeviceFeatures physical_device_features = {};
    // 	VkPhysicalDeviceFeatures requested_device_features = {};

    // swap_chain_create

    Error initialize(uint32_t p_device_index, uint32_t p_frame_count);
};

}