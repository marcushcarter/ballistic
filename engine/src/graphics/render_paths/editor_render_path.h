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

    bool resourcesReady = false;

    EditorRenderPath(Renderer& r, ImGuiLayer& i) : renderer(&r), imguiLayer(&i) {}

    bool CreateResources(Renderer& r) override {
        placeholderFeature.CreateResources(r);
        resourcesReady = true;
        return true;
    }

    void DestroyResources() override {
        placeholderFeature.DestroyResources();
        resourcesReady = false;
    }
    
    void Build(RenderGraph& g, FrameGraph& fg) override {
        if (resourcesReady) {
            placeholderFeature.AddPass(g, fg);
        }

        AddImGuiPass(g, fg, renderer, imguiLayer, resourcesReady);
    }
};
