// #include <core/rendering/render_path/features/post_processing_feature.h>

// namespace ballistic {

// Error PostProcessFeature::create_resources()
// {
//     using enum Error;

//     per_pixel_pass.name = "per_pixel_pp_pass";
//     per_pixel_pass.category = "PostProcess";
//     per_pixel_pass.setup = [](RenderGraph::Builder& b) {
//         b.color_attachment("final_image", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.1f, 0.2f, 0.8f, 1.0f } });

//         // b.read_image("final_image", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT);
          

//         // drivers::DeviceDriverVulkan::ImageCreateInfo depth_ci{};
//         // depth_ci.format = VK_FORMAT_D32_SFLOAT;
//         // depth_ci.usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//         // depth_ci.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
//         // depth_ci.sizing = drivers::DeviceDriverVulkan::ImageCreateInfo::Sizing::ViewportRelative;
//         // depth_ci.width_scale  = 1.0f;
//         // depth_ci.height_scale = 1.0f;
//         // b.create_image("test_depth", depth_ci);
//         // b.depth_attachment("test_depth", VK_ATTACHMENT_LOAD_OP_CLEAR, [] { VkClearValue v{}; v.depthStencil = { 1.0f, 0 }; return v; }());
        
//         // drivers::DeviceDriverVulkan::ImageCreateInfo image_ci2{};
//         // image_ci2.name = "test_color";
//         // image_ci2.format = VK_FORMAT_R8G8B8A8_UNORM;
//         // image_ci2.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//         // image_ci2.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
//         // image_ci2.sizing = drivers::DeviceDriverVulkan::ImageCreateInfo::Sizing::ViewportRelative;
//         // image_ci2.width_scale  = 1.0f;
//         // image_ci2.height_scale = 1.0f;
//         // b.create_image("test_color", image_ci2);
//         // b.color_attachment("test_color", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 1.0f, 0.0f, 1.0f } });
//     };
//     per_pixel_pass.execute = [](RenderGraph::CommandList& cl) {
//         (void)cl;
//     };

//     return Ok;
// };

// void PostProcessFeature::destroy_resources()
// {

// }

// void PostProcessFeature::build(RenderGraph& g)
// {
//     if (!enabled) return;
//     g.add(&per_pixel_pass);
// };

// }