#pragma once
#include "graphics/render_graph/render_graph.h"
struct Renderer;
ResourceHandle AddBlitToSwapchainPass(RenderGraph& g, Renderer* renderer);
