#pragma once
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <graphics/imgui_layer.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/passes/placeholder_pass.h>
#include <graphics/passes/imgui_pass.h>

struct ImGuiLayer;
struct Renderer;

struct EditorRenderPath : RenderPath
{
    Renderer* renderer = nullptr;
    ImGuiLayer* imguiLayer = nullptr;
    PlaceholderFeature placeholderFeature;

    EditorRenderPath(Renderer& r, ImGuiLayer& i) : renderer(&r), imguiLayer(&i) {}

    bool CreateResources(Renderer& r) override {
        placeholderFeature.CreateResources(r);
        return true;
    }

    void DestroyResources() override {
        placeholderFeature.DestroyResources();
    }
    
    void Build(RenderGraph& g) override {
        placeholderFeature.AddPass(g);
        AddImGuiPass(g, renderer, imguiLayer);
    }
};
