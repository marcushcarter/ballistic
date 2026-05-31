#include "editor_render_path.h"
#include "graphics/renderer.h"
#include "graphics/imgui_layer.h"
#include "graphics/render_graph/render_graph.h"
#include "graphics/passes/scene_placeholder_pass.h"
#include "graphics/passes/editor_imgui_pass.h"

void EditorRenderPath::Build(RenderGraph& g)
{
    AddScenePlaceholderPass(g);
    AddImGuiPass(g, renderer, imguiLayer);
}
