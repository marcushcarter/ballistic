#pragma once
#include "pch.h"
#include "vk/vk.h"

struct Renderer;

struct SplashRenderer
{
    Image2D logoImage;
    Image2D logoLongImage;

    DescriptorSet splashSet;
    PipelineLayout splashPipelineLayout;
    GraphicsPipeline splashPipeline;

    struct PushConstants {
        float x, y, w, h;
    };

    bool Create(Renderer& renderer);
    void Destroy();

    bool RenderLoadingFrame(Renderer& renderer);
};