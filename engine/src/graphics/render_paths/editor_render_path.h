#pragma once
#include <graphics/render_graph/render_path.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/renderer.h>
#include <graphics/imgui_layer.h>

#include <graphics/passes/scene_passes.h>
#include <graphics/passes/imgui_pass.h>

struct EditorRenderPath : RenderPath
{
    Renderer* renderer = nullptr;
    ImGuiLayer* imguiLayer = nullptr;

    ScenePasses scenePasses;
    bool resourcesReady = false;

    explicit EditorRenderPath(Renderer& r, ImGuiLayer& i) : renderer(&r), imguiLayer(&i) {}

    bool CreateResources(Renderer& r) override {
        scenePasses.CreateResources(r);
        resourcesReady = true;
        return true;
    }

    void DestroyResources() override {
        scenePasses.DestroyResources();
        resourcesReady = false;
    }
    
    void Build(RenderGraph& g, FrameGraph& fg) override {
        if (resourcesReady) {
            scenePasses.Build(g, fg);
        }

        AddImGuiPass(g, fg, renderer, imguiLayer, !resourcesReady);
    }
};
