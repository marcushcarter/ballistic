#include <drivers/vulkan/rendering_device_driver_vulkan.h>
#include <core/error/error.h>
#include <iostream>

namespace ballistic::drivers {

Error RenderingDeviceDriverVulkan::initialize(uint32_t p_device_index, uint32_t p_frame_count)
{
    using enum Error;
    Error err;
    (void)err;

    context_device = context_driver->device_get(p_device_index);
    physical_device = context_driver->physical_device_get(p_device_index);
	frame_count = p_frame_count;

    // std::cout << context_device.name.c_str() << " " << context_device.vendor_id << " " << context_device.device_type << "\n";
    
    // err = _initialize_vulkan_version();
	// BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    return Ok;
}

}