#include <core/rendering/render_path/features/subsurface_scattering_feature.h>

namespace ballistic {

static drivers::DeviceDriverVulkan::ImageCreateInfo sss_target_ci(const char* p_name = "")
{
    drivers::DeviceDriverVulkan::ImageCreateInfo ci{};
    ci.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    ci.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    ci.name = p_name;
    return ci;
}

Error SubsurfaceScatteringFeature::create_resources()
{
    using enum Error;

    sss_pass.name = "SssBlurH";
    sss_pass.category = "Lighting";
    sss_pass.setup = [](RenderGraph::Builder& b) {
        b.create_image("SssTmp", sss_target_ci("SssTmp"));
        b.write_image("SssTmp", VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
        b.read_image("HdrDiffuse", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_image("G_Depth", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_image("G_Rough_Metal", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    };
    sss_pass.execute = [](RenderGraph::CommandList& cl) {
        (void)cl;
    };

    composite_pass.name = "SssBlurVComposite";
    composite_pass.category = "Lighting";
    composite_pass.setup = [](RenderGraph::Builder& b) {
        b.create_image("HdrColor", sss_target_ci("HdrColor"));
        b.write_image("HdrColor", VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
        b.read_image("SssTmp", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_image("HdrSpecular", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_image("G_Depth", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_image("G_Rough_Metal", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    };
    composite_pass.execute = [](RenderGraph::CommandList& cl) {
        (void)cl;
    };

    return Ok;
};

void SubsurfaceScatteringFeature::destroy_resources() {}

void SubsurfaceScatteringFeature::build(RenderGraph& g)
{
    if (!enabled) return;
    g.add(&sss_pass);
    g.add(&composite_pass);
};

}