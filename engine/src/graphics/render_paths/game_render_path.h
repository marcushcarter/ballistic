#pragma once
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/passes/placeholder_pass.h>
#include <graphics/passes/blit_pass.h>

struct Renderer;

struct GameRenderPath : RenderPath
{
    Renderer* renderer = nullptr;
    PlaceholderFeature placeholderFeature;
    SwapchainBlitFeature blitFeature;

    explicit GameRenderPath(Renderer& r) : renderer(&r) {}

    bool CreateResources(Renderer& r) override {
        placeholderFeature.CreateResources(r);
        blitFeature.CreateResources(r);
        return true;
    }

    void DestroyResources() override {
        placeholderFeature.DestroyResources();
        blitFeature.DestroyResources();
    }
    
    void Build(RenderGraph& g) override {
        placeholderFeature.AddPass(g);
        blitFeature.AddPass(g);
    }
};
