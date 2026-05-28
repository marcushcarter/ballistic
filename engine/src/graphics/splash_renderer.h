#pragma once
#include "pch.h"
#include "vk/vk.h"

struct Renderer;

struct SplashRenderer
{
    Image2D logoImage;
    Image2D logoLongImage;

    DescriptorSetLayout splashSetLayout;
    DescriptorSet splashSet;
    PipelineLayout splashPipelineLayout;
    GraphicsPipeline splashPipeline;

    struct PushConstants {
        float x, y, w, h;
    };

    bool Create(Renderer& renderer);
    void Destroy();

    bool RenderLoadingFrame(Renderer& renderer);
    void RecordQuad(Renderer& renderer, VkCommandBuffer cmd, float xNorm, float yNorm, float wNorm, float hNorm);
};