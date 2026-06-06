#pragma once
#include <vk/vk.h>
struct Renderer;
struct RenderGraph;

struct SwapchainBlitFeature
{
    GraphicsPipeline pipeline;
    Renderer* renderer = nullptr;

    bool CreateResources(Renderer& r);
    void DestroyResources();
    void AddPass(RenderGraph& g);
};
