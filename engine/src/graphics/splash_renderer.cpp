#include "splash_renderer.h"
#include "renderer.h"

bool SplashRenderer::Create(Renderer& renderer)
{
    // TODO: move from Renderer::RecreateImage
    (void)renderer;
    return false;
}

void SplashRenderer::Destroy()
{
    // TODO: move from Renderer::DestroyImage
}

bool SplashRenderer::RenderLoadingFrame(Renderer& renderer)
{
    // TODO: move from Renderer::UnloadProject
    (void)renderer;
    return false;
}

// void RenderGraphResources::OnViewportResized(Renderer& renderer, uint32_t newW, uint32_t newH)
// {
//     // TODO: move the viewport-relative resize loop from Renderer::ViewportResize
// }
