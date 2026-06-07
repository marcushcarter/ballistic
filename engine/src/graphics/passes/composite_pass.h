#pragma once
#include <vk/vk.h>
struct Renderer;
struct RenderGraph;
struct FrameGraph;

struct CompositePass
{
    Renderer* renderer = nullptr;

    bool CreateResources(Renderer& r);
    void DestroyResources();
    void AddPass(RenderGraph& g, FrameGraph& fg);
};
