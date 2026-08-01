#include <editor/editor_logos.h>
#include <core/io/image_io.h>

namespace ballistic {
    
Error EditorLogos::initialize(drivers::DeviceDriverVulkan& r_dd)
{
    using enum Error;

    dd = &r_dd;
    
    ImageData<uint8_t, 4> icon_data = ImageIO::load_from_resource<uint8_t, 4>(L"LOGOS_ICON_PNG");
    if (icon_data.valid()) icon_image = dd->image_create_texture(icon_data.pixels, static_cast<uint32_t>(icon_data.width), static_cast<uint32_t>(icon_data.height), "editor_logo");
    ImageIO::free_image(icon_data);

    ImageData<uint8_t, 4> logo_data = ImageIO::load_from_resource<uint8_t, 4>(L"LOGOS_ICON_PNG");
    if (logo_data.valid()) logo_image = dd->image_create_texture(logo_data.pixels, static_cast<uint32_t>(logo_data.width), static_cast<uint32_t>(logo_data.height), "editor_logo");
    ImageIO::free_image(logo_data);

    return Ok;
}

void EditorLogos::shutdown()
{
    dd->device_wait_idle();

    dd->image_free(icon_image);
    dd->image_free(logo_image);
}

}