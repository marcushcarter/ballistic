#pragma once
#include "pch.h"
#include "vk/vk.h"
#include "project/render_graph.h"

struct Renderer;

struct RenderGraphResources
{
    struct AllocatedImage {
        Image2D image;
        bool viewportRelative = false;
        float relativeWidth = 1.0f;
        float relativeHeight = 1.0f;
    };

    std::unordered_map<uint64_t, AllocatedImage> images;

    bool RecreateImage(Renderer& renderer, const RGImage& desc);
    void DestroyImage(Renderer& renderer, uint64_t id);
    void DestroyAll(Renderer& renderer);

    void OnViewportResized(Renderer& renderer, uint32_t newWidth, uint32_t newHeight);
};