#pragma once
#include "pch.h"

struct Renderer;
struct Scene;
struct RenderGraph;

struct RenderPath
{
    // Renderer* renderer = nullptr;
    // Scene* scene = nullptr;

    virtual ~RenderPath() = default;
    virtual void Build(RenderGraph& graph) = 0;
};
