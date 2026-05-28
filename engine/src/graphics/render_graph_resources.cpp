#include "render_graph_resources.h"

bool RenderGraphResources::RecreateImage(Renderer& renderer, const RGImage& desc)
{   
    // TODO: move logo/logoLong load + splash pipeline create from Renderer::Start
    (void)renderer;
    (void)desc;
    return false;
}

void RenderGraphResources::DestroyImage(Renderer& renderer, uint64_t id)
{
    // TODO
    (void)renderer;
    (void)id;
}

void RenderGraphResources::DestroyAll(Renderer& renderer)
{
    // TODO: move from Renderer::RenderLoadingScreen
    (void)renderer;
}

void RenderGraphResources::OnViewportResized(Renderer& renderer, uint32_t newWidth, uint32_t newHeight)
{
    // TODO: extract the splash draw GameApplication currently inlines
    (void)renderer;
    (void)newWidth;
    (void)newHeight;
}
