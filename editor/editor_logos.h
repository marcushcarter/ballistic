#pragma once
#include <drivers/vulkan/device_driver_vulkan.h>
#include <core/base/error.h>

namespace ballistic {

struct EditorLogos
{
    drivers::DeviceDriverVulkan* dd = nullptr;
    
    drivers::DeviceDriverVulkan::Image icon_image;
    drivers::DeviceDriverVulkan::Image logo_image;
    
    Error initialize(drivers::DeviceDriverVulkan& r_dd);
    void shutdown();
};

}