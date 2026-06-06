#pragma once
#include <vk/vk.h>
struct Renderer;
struct RenderGraph;

struct PlaceholderFeature
{
    bool CreateResources(Renderer& renderer);
    void DestroyResources();
    void AddPass(RenderGraph& g);
};
