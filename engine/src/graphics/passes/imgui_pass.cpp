#include <graphics/passes/imgui_pass.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <graphics/imgui_layer.h>
#include <core/log.h>

void AddImGuiPass(RenderGraph& g, FrameGraph& fg, Renderer* renderer, ImGuiLayer* imguiLayer, bool hasProject)
{    
    struct PassData { ResourceHandle swapchain; };
    PassData out = g.AddPass<PassData>("ImGuiPass",
    [&](RenderGraph& builder, PassData& data) {
        if (hasProject) {
            builder.ReadImage(fg.finalImage,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT);
        }

        data.swapchain = builder.WriteImage("Swapchain",
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        fg.swapchain = data.swapchain;
    },
    [renderer, imguiLayer](VkCommandBuffer cmd, const PassData& data, RenderGraph& g) {
        VkImageView view = g.GetImageView(data.swapchain);
        VkExtent2D ext = g.GetImageExtent(data.swapchain);
        if (!view) return;

        VkRenderingAttachmentInfo color{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color.imageView = view;
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

        VkRenderingInfo info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        info.renderArea = { {0,0}, ext };
        info.layerCount = 1;
        info.colorAttachmentCount = 1;
        info.pColorAttachments = &color;

        vkCmdBeginRendering(cmd, &info);
        imguiLayer->RecordDraw(cmd);
        vkCmdEndRendering(cmd);
    });
}
