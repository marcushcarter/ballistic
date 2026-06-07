#pragma once
#include <graphics/render_graph/render_graph.h>

struct Renderer;
struct RenderGraph;

struct GBufferOutput { ResourceHandle albedo, normal, material; };

struct FrameGraph {
    // Imported
    ResourceHandle finalImage;
    ResourceHandle swapchain;
    ResourceHandle frameUniform;

    ResourceHandle mainZBuffer;
    GBufferOutput gbuffer;
    ResourceHandle aoRaw;
    ResourceHandle aoBlurred;
    
    ResourceHandle lightDiffuseImage;
    ResourceHandle lightDiffuseSSSImage;
    ResourceHandle ligthSpecularImage;
    ResourceHandle lightImage;
};

struct RenderPath
{
    virtual ~RenderPath() = default;
    virtual bool CreateResources(Renderer& r) = 0;
    virtual void DestroyResources() = 0;
    virtual void Build(RenderGraph& g, FrameGraph& fg) = 0;
};
