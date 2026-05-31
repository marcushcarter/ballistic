#pragma once
#include "pch.h"
#include "graphics/render_graph/render_path.h"

struct ImGuiLayer;
struct Renderer;

struct EditorRenderPath : RenderPath
{
    Renderer* renderer = nullptr;
    ImGuiLayer* imguiLayer = nullptr;
    EditorRenderPath(Renderer& r, ImGuiLayer& i) : renderer(&r), imguiLayer(&i) {}
    void Build(RenderGraph& g) override;
};
