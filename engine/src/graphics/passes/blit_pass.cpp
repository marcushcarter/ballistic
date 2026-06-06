#include <graphics/passes/blit_pass.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <core/assert.h>
#include <resources.h>

bool SwapchainBlitFeature::CreateResources(Renderer& r)
{
    renderer = &r;
    
    Shader vert{}, frag{};
    BE_ASSERT(vert.LoadOrCompile(r.device.Get(), VK_SHADER_STAGE_VERTEX_BIT, LoadShaderSource(SHADER_FULLSCREEN_VERT), r.projectPath / ".ballistic/cache/shaders/fullscreen.vert.spv", "fullscreen.vert"));    
    BE_ASSERT(frag.LoadOrCompile(r.device.Get(), VK_SHADER_STAGE_FRAGMENT_BIT, LoadShaderSource(SHADER_BLIT_FRAG), r.projectPath / ".ballistic/cache/shaders/blit.frag.spv", "blit.frag"));

    PipelineRenderingInfo renderingInfo;
    renderingInfo.colorFormats = { r.swapchain.format };
    auto renderingCreateInfo = renderingInfo.Get();

    BE_ASSERT(pipeline.Create(r.device.Get(), {
        .pNext = &renderingCreateInfo,
        .layout = r.globalPipelineLayout.Get(),
        .cache = r.pipelineCache.Get(),
        .shaderStages = { PipelineShaderStage(vert.Get(), vert.stage), PipelineShaderStage(frag.Get(), frag.stage) },
        .debugName = "BlitPipeline"
    }));

    vert.Destroy();
    frag.Destroy();
    return true;
}

void SwapchainBlitFeature::DestroyResources()
{
    pipeline.Destroy();
}

void SwapchainBlitFeature::AddPass(RenderGraph& g, FrameGraph& fg)
{
    struct PassData { ResourceHandle src, dst, frameUn; };
    PassData out = g.AddPass<PassData>("SwapchainBlitPass",
    [&](RenderGraph& builder, PassData& data) {
        data.frameUn = builder.ReadBuffer(fg.frameUniform, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);
        data.src = builder.ReadImage(fg.finalImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);
        data.dst = builder.WriteImage(fg.swapchain, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
        fg.swapchain = data.dst;
    },
    [this](VkCommandBuffer cmd, const PassData& data, RenderGraph& g) {    
        VkImageView swapView = g.GetImageView(data.dst);
        VkExtent2D ext = g.GetImageExtent(data.dst);
        if (!swapView) return;

        VkRenderingAttachmentInfo color{};
        color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        color.imageView = swapView;
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

        VkRenderingInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        info.renderArea = { {0,0}, ext };
        info.layerCount = 1;
        info.colorAttachmentCount = 1;
        info.pColorAttachments = &color;

        vkCmdBeginRendering(cmd, &info);

        ViewportScissor(cmd, 0, 0, (float)ext.width, (float)ext.height);
        pipeline.Bind(cmd);
        struct { uint32_t srcIndex, samplerIndex; VkDeviceAddress address; } pc;
        pc.srcIndex = g.GetBindlessSampled(data.src);
        pc.samplerIndex = renderer->linearSampler.bindlessSampler;
        pc.address = g.GetDeviceAddress(data.frameUn);
        vkCmdPushConstants(cmd, renderer->globalPipelineLayout.Get(), VK_SHADER_STAGE_ALL, 0, sizeof(pc), &pc);
        vkCmdDraw(cmd, 3, 1, 0, 0);

        vkCmdEndRendering(cmd);
    });
}
