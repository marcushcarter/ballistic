#pragma once
#include "graphics/render_graph/render_graph.h"
struct Renderer;
struct ImGuiLayer;
ResourceHandle AddImGuiPass(RenderGraph& g, Renderer* renderer, ImGuiLayer* imguiLayer);
