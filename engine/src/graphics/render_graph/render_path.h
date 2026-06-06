#pragma once

struct Renderer;
struct Scene;
struct RenderGraph;

struct RenderPath
{
    virtual ~RenderPath() = default;
    virtual bool CreateResources(Renderer& renderer) = 0;
    virtual void DestroyResources() = 0;
    virtual void Build(RenderGraph& graph) = 0;
};
