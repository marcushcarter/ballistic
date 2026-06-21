#pragma once
#include <drivers/vulkan/rendering_device_driver_vulkan.h>
#include <core/error/error.h>
// #include <vector>

namespace ballistic {

struct Renderer
{
    drivers::RenderingDeviceDriverVulkan* device_driver = nullptr;

    Error create(uint32_t p_frame_count);
    void destroy();

    Error begin_frame();
    Error end_frame();
};

}