#include <core/rendering/render_path/features/deferred_lighting_feature.h>

namespace ballistic {

static drivers::DeviceDriverVulkan::ImageCreateInfo hdr_target_ci(const char* p_name = "")
{
    drivers::DeviceDriverVulkan::ImageCreateInfo ci{};
    ci.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    ci.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    ci.name = p_name;
    return ci;
}

Error DeferredLightingFeature::create_resources()
{
    using enum Error;

    lighting_pass.name = "Lighting";
    lighting_pass.category = "Lighting";
    lighting_pass.setup = [](RenderGraph::Builder& b) {  
        b.create_image("HdrDiffuse", hdr_target_ci("HdrDiffuse"));
        b.create_image("HdrSpecular", hdr_target_ci("HdrSpecular"));
        b.write_image("HdrDiffuse", VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
        b.write_image("HdrSpecular", VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
        
        b.read_image("G_Albedo", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_image("G_Normal", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_image("G_Rough_Metal", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_image("G_Motion", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_image("G_Depth", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_image("AmbientOcclusion", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        // shadow maps
    };
    lighting_pass.execute = [](RenderGraph::CommandList& cl) {
        (void)cl;
    };

    return Ok;
};

void DeferredLightingFeature::destroy_resources() {}

void DeferredLightingFeature::build(RenderGraph& g)
{
    if (!enabled) return;
    g.add(&lighting_pass);
};

}