#include <graphics/passes/placeholder_pass.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <core/assert.h>
#include <resources.h>

bool PlaceholderPass::CreateResources(Renderer& r)
{
    renderer = &r;
    
    Shader vert{}, frag{};
    BE_ASSERT(vert.LoadOrCompile(r.device.Get(), VK_SHADER_STAGE_VERTEX_BIT, LoadShaderSource(SHADER_TRIANGLE_VERT), r.projectPath / ".ballistic/cache/shaders/triangle.vert.spv", "triangle.vert"));    
    BE_ASSERT(frag.LoadOrCompile(r.device.Get(), VK_SHADER_STAGE_FRAGMENT_BIT, LoadShaderSource(SHADER_TRIANGLE_FRAG), r.projectPath / ".ballistic/cache/shaders/triangle.frag.spv", "triangle.frag"));

    PipelineRenderingInfo renderingInfo;
    renderingInfo.colorFormats = { r.finalImage.format };
    auto renderingCreateInfo = renderingInfo.Get();

    BE_ASSERT(pipeline.Create(r.device.Get(), {
        .pNext = &renderingCreateInfo,
        .layout = r.globalPipelineLayout.Get(),
        .cache = r.pipelineCache.Get(),
        .shaderStages = { PipelineShaderStage(vert.Get(), vert.stage), PipelineShaderStage(frag.Get(), frag.stage) },
        .debugName = "TrianglePipeline"
    }));

    vert.Destroy();
    frag.Destroy();
    return true;
}

void PlaceholderPass::DestroyResources()
{
    pipeline.Destroy();
}

void PlaceholderPass::AddPass(RenderGraph& g, FrameGraph& fg)
{
    struct PassData { ResourceHandle dst; };
    PassData out = g.AddPass<PassData>("PlaceholderPass",
    [&](RenderGraph& builder, PassData& data)
    {
        // builder.ReadImage(fg.lightImage,
        // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        // VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        // VK_ACCESS_2_SHADER_READ_BIT);
        
        builder.ReadImage(fg.mainZBuffer,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT);

        data.dst = builder.WriteImage(fg.finalImage,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        fg.finalImage = data.dst;
    },
    [this](VkCommandBuffer cmd, const PassData& data, RenderGraph& g)
    {
        VkExtent2D ext = g.GetImageExtent(data.dst);

        VkRenderingAttachmentInfo color{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color.imageView = g.GetImageView(data.dst);
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.clearValue.color = { 1.0f, 1.0f, 1.0f, 1.0f };

        VkRenderingInfo info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        info.renderArea = { {0,0}, ext };
        info.layerCount = 1;
        info.colorAttachmentCount = 1;
        info.pColorAttachments = &color;

        vkCmdBeginRendering(cmd, &info);
        ViewportScissor(cmd, 0, 0, (float)ext.width, (float)ext.height);
        pipeline.Bind(cmd);
        vkCmdDraw(cmd, 3, 1, 0, 0);
        vkCmdEndRendering(cmd);
    });

}
