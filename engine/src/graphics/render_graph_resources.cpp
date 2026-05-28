#include "render_graph_resources.h"
#include "renderer.h"
#include "project/project.h"

static VkImageAspectFlags ResolveAspect(VkFormat format)
{
    switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

static VkImageUsageFlags ResolveUsage(const RGImage& desc)
{
    VkImageUsageFlags flags = 0;
    if (desc.usageAttachment) {
        bool isDepth = desc.format == VK_FORMAT_D16_UNORM || desc.format == VK_FORMAT_D32_SFLOAT || desc.format == VK_FORMAT_X8_D24_UNORM_PACK32 || desc.format == VK_FORMAT_D16_UNORM_S8_UINT || desc.format == VK_FORMAT_D24_UNORM_S8_UINT || desc.format == VK_FORMAT_D32_SFLOAT_S8_UINT;
        flags |= isDepth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (desc.usageSampled) flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (desc.usageStorage) flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    return flags;
}

bool RenderGraphResources::RecreateImage(Renderer& renderer, const RGImage& desc)
{
    renderer.device.Wait();

    auto it = images.find(desc.id);
    if (it != images.end()) {
        it->second.image.Destroy();
        images.erase(it);
    }

    AllocatedImage entry;
    entry.viewportRelative = desc.sizeMode == RGImageSizeMode::ViewportRelative;
    entry.relativeWidth = desc.relativeWidth;
    entry.relativeHeight = desc.relativeHeight;

    VkExtent2D extent = entry.viewportRelative
        ? VkExtent2D{ renderer.finalImage.extent.width, renderer.finalImage.extent.height }
        : VkExtent2D{ desc.fixedWidth, desc.fixedHeight };

    bool ok = entry.image.Create(renderer.device.Get(), renderer.physicalDevice.memory, {
        .extent = extent,
        .format = desc.format,
        .usage = ResolveUsage(desc),
        .aspect = ResolveAspect(desc.format),
        .debugName = desc.name.c_str()
    });

    if (!ok) {
        LOG_ERROR("RecreateImage failed: %s", desc.name.c_str());
        return false;
    }

    images[desc.id] = std::move(entry);
    LOG_DEBUG("Allocated image: %s [%llu]", desc.name.c_str(), desc.id);
    return true;
}

void RenderGraphResources::DestroyImage(uint64_t id)
{
    auto it = images.find(id);
    if (it != images.end()) {
        it->second.image.Destroy();
        images.erase(it);
    }
}

void RenderGraphResources::DestroyAll()
{
    for (auto& [id, entry] : images) entry.image.Destroy();
    images.clear();
    LOG_DEBUG("Project Vulkan objects unloaded");
}

bool RenderGraphResources::LoadProject(Renderer& renderer, const Project& project)
{
    for (auto& desc : project.images) RecreateImage(renderer, desc);
    LOG_DEBUG("Project Vulkan objects loaded");
    return true;
}

void RenderGraphResources::ViewportResize(uint32_t newWidth, uint32_t newHeight)
{
    for (auto& [id, entry] : images) {
        if (!entry.viewportRelative) continue;
        entry.image.Resize({ (uint32_t)(newWidth * entry.relativeWidth), (uint32_t)(newHeight * entry.relativeHeight) });
    }
}
