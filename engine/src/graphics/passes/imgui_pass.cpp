#include <graphics/passes/imgui_pass.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/renderer.h>
#include <graphics/imgui_layer.h>

void AddImGuiPass(RenderGraph& g, Renderer* renderer, ImGuiLayer* imguiLayer)
{    
    struct PassData { ResourceHandle dst; };
    PassData out = g.AddPass<PassData>("ScenePlaceholderPass",
    [&](RenderGraph& builder, PassData& data) {
        builder.ReadImage("finalImage", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);
        data.dst = builder.WriteImage("swapchain", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
    },
    [renderer, imguiLayer](VkCommandBuffer cmd, const PassData& data, RenderGraph& g) {
        VkImageView view = g.GetImageView(data.dst);
        VkExtent2D ext = g.GetImageExtent(data.dst);
        if (!view) return;

        VkRenderingAttachmentInfo color{};
        color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        color.imageView = view;
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.clearValue.color = { 0.1f, 0.0f, 1.0f, 1.0f };

        VkRenderingInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        info.renderArea = { {0,0}, ext };
        info.layerCount = 1;
        info.colorAttachmentCount = 1;
        info.pColorAttachments = &color;

        vkCmdBeginRendering(cmd, &info);
        imguiLayer->RecordDraw(cmd);
        vkCmdEndRendering(cmd);
    });
}
