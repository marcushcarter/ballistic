#include "editor_render_path.h"
#include "graphics/renderer.h"
#include "graphics/imgui_layer.h"
#include "graphics/render_graph/render_graph.h"
#include "graphics/passes/scene_placeholder_pass.h"
#include "graphics/passes/editor_imgui_pass.h"

void EditorRenderPath::Build(RenderGraph& g)
{
    struct PassData { ResourceHandle dst; };
    PassData out = g.AddPass<PassData>("Pass New",
    [&](RenderGraph& builder, PassData& data) {

        (void)data;

        builder.CreateBuffer("MyBuffer", {
            .size  = sizeof(float) * 1024,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        },
        VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
    },
    [](VkCommandBuffer cmd, const PassData& data, RenderGraph& g) {
        (void)cmd;
        (void)data;
        (void)g;
    });

    AddScenePlaceholderPass(g);
    AddImGuiPass(g, renderer, imguiLayer);
}
