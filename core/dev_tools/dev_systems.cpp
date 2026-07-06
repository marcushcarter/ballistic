#include <core/dev_tools/dev_systems.h>
#include <core/rendering/renderer.h>

namespace ballistic {

Error DevSystems::create(Renderer& r_renderer, drivers::DeviceDriverVulkan& r_device_driver)
{
    renderer = &r_renderer;
    device_driver = &r_device_driver;
    texture_cache.create(r_device_driver);
    return Error::Ok;
}

void DevSystems::destroy()
{
    texture_cache.destroy();
}

void DevSystems::begin_frame()
{
    texture_cache.begin_frame(renderer->frame_number, renderer->frame_count, renderer->resize_epoch);
}

void DevSystems::end_frame()
{
    texture_cache.collect(renderer->frame_number);
}

}