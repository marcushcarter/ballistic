#include "gpu_resources.h"
#include "renderer.h"
#include "project/project.h"

static bool IsDepthFormat(VkFormat f)
{
    switch (f) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

static VkImageAspectFlags ResolveAspect(VkFormat f)
{
    switch (f) {
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

static VkImageUsageFlags ResolveImageUsage(const ResourceImageDesc& d)
{
    VkImageUsageFlags flags = 0;
    if (d.usageAttachment) {
        flags |= IsDepthFormat(d.format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (d.usageSampled) flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (d.usageStorage) flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    return flags;
}

bool GpuResources::CreateImage(Renderer& renderer, const ResourceImageDesc& desc)
{    
    renderer.device.Wait();
    DestroyImage(desc.id);

    VkExtent2D extent = desc.sizeMode == ImageSizeMode::ViewportRelative
        ? VkExtent2D{ (uint32_t)(renderer.finalImage.extent.width  * desc.relativeWidth), (uint32_t)(renderer.finalImage.extent.height * desc.relativeHeight) }
        : VkExtent2D{ desc.fixedWidth, desc.fixedHeight };

    ImageEntry entry;
    entry.desc = desc;

    bool ok = entry.image.Create(
        renderer.device.Get(), 
        renderer.physicalDevice.memory, 
        {
            .extent = extent,
            .format = desc.format,
            .usage = ResolveImageUsage(desc),
            .aspect = ResolveAspect(desc.format),
            .debugName = desc.name.c_str()
        }
    );

    if (!ok) {
        LOG_ERROR("CreateImage failed: %s", desc.name.c_str());
        return false;
    }

    images[desc.id] = std::move(entry);
    LOG_DEBUG("Allocated image: %s [%llu]", desc.name.c_str(), desc.id);
    return true;
}

void GpuResources::DestroyImage(uint64_t id)
{
    auto it = images.find(id);
    if (it == images.end()) return;
    it->second.image.Destroy();
    images.erase(it);
}

Image2D* GpuResources::GetImage(uint64_t id)
{
    auto it = images.find(id);
    return it == images.end() ? nullptr : &it->second.image;
}

void GpuResources::DestroyAll()
{
    for (auto& [id, e] : images)  e.image.Destroy();
    images.clear();
    LOG_DEBUG("Project GPU objects unloaded");
}

bool GpuResources::LoadProject(Renderer& renderer, const Project& project)
{
    for (auto& d : project.images) CreateImage(renderer, d);
    LOG_DEBUG("Project GPU objects loaded");
    return true;
}

void GpuResources::ResizeViewport(uint32_t width, uint32_t height)
{
    for (auto& [id, e] : images) {
        if (e.desc.sizeMode != ImageSizeMode::ViewportRelative) continue;
        e.image.Resize({ (uint32_t)(width  * e.desc.relativeWidth), (uint32_t)(height * e.desc.relativeHeight) });
    }
}
