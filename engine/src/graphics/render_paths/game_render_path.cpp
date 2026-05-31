#include "game_render_path.h"
#include "graphics/renderer.h"
#include "graphics/render_graph/render_graph.h"
#include "graphics/passes/scene_placeholder_pass.h"
#include "graphics/passes/blit_to_swapchain_pass.h"

void GameRenderPath::Build(RenderGraph& g)
{
    AddScenePlaceholderPass(g);
    AddBlitToSwapchainPass(g, renderer);
}
