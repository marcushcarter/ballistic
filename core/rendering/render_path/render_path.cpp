#include <core/rendering/render_path/render_path.h>
#include <core/log/log.h>

namespace ballistic {

Error RenderPath::create_resources()
{
    using enum Error;
    
    temp_pass.name = "temp";
    temp_pass.setup = [](RenderGraph::Builder& b) {
        b.color_attachment("final_image", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.1f, 0.2f, 0.8f, 1.0f } });
        
        

        drivers::DeviceDriverVulkan::ImageCreateInfo depth_ci{};
        depth_ci.format = VK_FORMAT_D32_SFLOAT;
        depth_ci.usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        depth_ci.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
        depth_ci.sizing = drivers::DeviceDriverVulkan::ImageCreateInfo::Sizing::ViewportRelative;
        depth_ci.width_scale  = 1.0f;
        depth_ci.height_scale = 1.0f;
        b.create_image("depth222", depth_ci);
        b.depth_attachment("depth222", VK_ATTACHMENT_LOAD_OP_CLEAR, [] { VkClearValue v{}; v.depthStencil = { 1.0f, 0 }; return v; }());
        
        drivers::DeviceDriverVulkan::ImageCreateInfo image_ci2{};
        image_ci2.name = "test_color";
        image_ci2.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_ci2.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        image_ci2.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        image_ci2.sizing = drivers::DeviceDriverVulkan::ImageCreateInfo::Sizing::ViewportRelative;
        image_ci2.width_scale  = 1.0f;
        image_ci2.height_scale = 1.0f;
        b.create_image("color222", image_ci2);
        // image_2 = device_driver->image_create_dedicated(image_ci2, { width, height, 1 });
        b.color_attachment("color222", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 1.0f, 0.0f, 1.0f } });
    };

    temp_pass.execute = [](VkCommandBuffer cmd, RenderGraph& g) {
        (void)cmd;
        (void)g;
    };

    return Ok;
}

void RenderPath::destroy_resources() {}

void RenderPath::build(RenderGraph& g)
{
    g.add(&temp_pass);
    build_present(g);
}

}