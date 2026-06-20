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

};

}