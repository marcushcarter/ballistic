#pragma once
#include "pch.h"
#include "vk/vk.h"
#include "project/resource_desc.h"

struct Renderer;
struct Project;

struct GpuResources
{
    struct ImageEntry { Image2D image; ResourceImageDesc desc; };

    std::unordered_map<uint64_t, ImageEntry> images;

    bool CreateImage(Renderer& renderer, const ResourceImageDesc& desc);
    void DestroyImage(uint64_t id);
    Image2D* GetImage(uint64_t id);

    void DestroyAll();

    bool LoadProject(Renderer& renderer, const Project& project);

    void ResizeViewport(uint32_t width, uint32_t height);
};