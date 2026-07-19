#include <core/rendering/render_path/features/geometry_feature.h>

namespace ballistic {

Error GeometryFeature::create_resources()
{
    using enum Error;

    geometry_pass.name = "Deferred";
    geometry_pass.category = "Geometry";
    geometry_pass.formats = {
        { VK_FORMAT_R8G8B8A8_SRGB },
        { VK_FORMAT_R16G16_UNORM },
        { VK_FORMAT_R8G8B8A8_UNORM },
        { VK_FORMAT_R16G16_SFLOAT },
        { VK_FORMAT_D32_SFLOAT, true },
    };
    geometry_pass.setup = [](RenderGraph::Builder& b) {
        drivers::DeviceDriverVulkan::ImageCreateInfo albedo_image_ci{};
        albedo_image_ci.name = "G_Albedo";
        albedo_image_ci.format = VK_FORMAT_R8G8B8A8_SRGB;
        albedo_image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        b.create_image("G_Albedo", albedo_image_ci);

        drivers::DeviceDriverVulkan::ImageCreateInfo normal_image_ci{};
        normal_image_ci.name = "G_Normal";
        normal_image_ci.format = VK_FORMAT_R16G16_UNORM;
        normal_image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        b.create_image("G_Normal", normal_image_ci);

        drivers::DeviceDriverVulkan::ImageCreateInfo rough_met_image_ci{};
        rough_met_image_ci.name = "G_Rough_Metal";
        rough_met_image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
        rough_met_image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        b.create_image("G_Rough_Metal", rough_met_image_ci);

        drivers::DeviceDriverVulkan::ImageCreateInfo motion_image_ci{};
        motion_image_ci.name = "G_Motion";
        motion_image_ci.format = VK_FORMAT_R16G16_SFLOAT;
        motion_image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        b.create_image("G_Motion", motion_image_ci);

        b.color_attachment("G_Albedo", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 1.0f, 0.0f, 1.0f } });
        b.color_attachment("G_Normal", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 1.0f, 0.0f, 1.0f } });
        b.color_attachment("G_Rough_Metal", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 1.0f, 0.0f, 1.0f } });
        b.color_attachment("G_Motion", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 1.0f, 0.0f, 1.0f } });
        b.depth_attachment_read("G_Depth", VK_ATTACHMENT_LOAD_OP_LOAD);

        drivers::DeviceDriverVulkan::BufferCreateInfo test_buffer_ci{};
        test_buffer_ci.name = "G_TestBuffer";
        test_buffer_ci.size = 3 * 1024 * 1024;
        test_buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        test_buffer_ci.memory = drivers::DeviceDriverVulkan::BufferCreateInfo::Memory::DeviceLocal;
        b.create_buffer("G_TestBuffer", test_buffer_ci);
        b.create_buffer("G_TestBuffer2", test_buffer_ci);
        b.write_buffer("G_TestBuffer", VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
        b.write_buffer("G_TestBuffer2", VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
    };
    geometry_pass.execute = [](RenderGraph::CommandList& cl) {
        (void)cl;
    };

    return Ok;
};

void GeometryFeature::destroy_resources() {}

void GeometryFeature::build(RenderGraph& g)
{
    if (!enabled) return;
    g.add(&geometry_pass);
};

}