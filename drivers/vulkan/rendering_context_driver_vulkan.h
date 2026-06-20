#pragma once

#include <drivers/vulkan/ballistic_vulkan.h>

namespace ballistic::drivers {

struct RenderingContextDriverVulkan
{
    // instance
    // extensions
    // devices (vector)
    // physical devices (vector)
    // queue families
    // debug messenger

    // initialize_vulkan_version
    // void _register_requested_instance_extension(const CharString &p_extension_name, bool p_required); // for next function, adds the extension into the required extensions hash map, helper
    // init instance extensions
    // init instances (instance and debug messenger and physical devices)
    // init devices

    // full initialize function (combo of above)

    // get_instance
    // get_physical_device
    // etc

    // debug messenger callback function

    // surface struct (useage is in device driver)
};

}