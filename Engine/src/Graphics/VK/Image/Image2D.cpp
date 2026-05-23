#include "Image2D.h"

inline uint32_t FindMemoryType(const VkPhysicalDeviceMemoryProperties& props, uint32_t typeFilter, VkMemoryPropertyFlags requiredFlags)
{
    for (uint32_t i = 0; i < props.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (props.memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}

bool Image2D::Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& props, const Image2DDesc& desc)
{
    VK_CHECK_HANDLE(device, VkDevice);

    Destroy();
    extent = desc.extent;
    format = desc.format;
    usage = desc.usage;
    aspect = desc.aspect;
    mipLevels = desc.mipLevels;
    layers = desc.layers;
    debugName = desc.debugName;
    stage = 0;
    access = 0;
    layout = VK_IMAGE_LAYOUT_UNDEFINED;
    deviceHandle = device;
    memoryProps = &props;
    ownsImage = true;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = { extent.width, extent.height, 1 };
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = layers;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        LOG_ERROR("Image2D create failed: %s - vkCreateImage", debugName ? debugName : "Unnamed");
        return false;
    }

    VkMemoryRequirements memReq{};
    vkGetImageMemoryRequirements(device, image, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = FindMemoryType(props, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        LOG_ERROR("Image2D create failed: %s - vkAllocateMemory", debugName ? debugName : "Unnamed");
        return false;
    }

    if (vkBindImageMemory(device, image, memory, 0) != VK_SUCCESS) {
        LOG_ERROR("Image2D create failed: %s - vkBindImageMemory", debugName ? debugName : "Unnamed");
        return false;
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = (layers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    viewInfo.subresourceRange.aspectMask = aspect;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = layers;

    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        LOG_ERROR("Image2D create failed: %s - vkCreateImageView", debugName ? debugName : "Unnamed");
        return false;
    }

    if (debugName) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
        nameInfo.objectHandle = (uint64_t)image;
        nameInfo.pObjectName = debugName;
        auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
        if (func) func(device, &nameInfo);
    }
    
    LOG_DEBUG("Image2D created: %s (%ux%u, %s, usage %s)",
        debugName ? debugName : "Unnamed",
        extent.width, extent.height,
        vk::to_string(vk::Format(format)).c_str(),
        vk::to_string(vk::ImageUsageFlags(usage)).c_str()
    );

    return true;
}

bool Image2D::WrapSwapchainImage(VkDevice device, VkImage swapchainImage, VkFormat swapchainFormat, VkExtent2D swapchainExtent)
{
    VK_CHECK_HANDLE(device, VkDevice);
    VK_CHECK_HANDLE(swapchainImage, VkImage);

    Destroy();
    image = swapchainImage;
    format = swapchainFormat;
    extent = swapchainExtent;
    debugName = "Swapchain Image";
    stage = 0;
    access = 0;
    layout = VK_IMAGE_LAYOUT_UNDEFINED;
    aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    deviceHandle = device;
    ownsImage = false;

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = (layers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    viewInfo.subresourceRange.aspectMask = aspect;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = layers;

    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        LOG_ERROR("Image2D create failed: %s - vkCreateImageView", debugName ? debugName : "Unnamed");
        return false;
    }

    if (debugName) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
        nameInfo.objectHandle = (uint64_t)image;
        nameInfo.pObjectName = debugName;
        auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
        if (func) func(device, &nameInfo);
    }

    LOG_DEBUG("Swapchain Image2D wrapped: (%ux%u, %s)",
        extent.width, extent.height,
        vk::to_string(vk::Format(format)).c_str()
    );
    
    return true;
}

bool Image2D::Resize(VkExtent2D newExtent)
{
    if (!ownsImage) {
        LOG_WARN("Image2D resize failed: %s - Cannot resize image that does not own its memory", debugName ? debugName : "Unnamed");
        return false;
    }

    return Create(deviceHandle, *memoryProps, {
        .extent = newExtent,
        .format = format,
        .usage = usage,
        .aspect = aspect,
        .mipLevels = mipLevels,
        .layers = layers,
        .debugName = debugName
    });
}

void Image2D::Destroy()
{
    if (imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(deviceHandle, imageView, nullptr);
        imageView = VK_NULL_HANDLE;
    }

    if (image != VK_NULL_HANDLE && ownsImage) {
        vkDestroyImage(deviceHandle, image, nullptr);
        vkFreeMemory(deviceHandle, memory, nullptr);
        image = VK_NULL_HANDLE;
        memory = VK_NULL_HANDLE;
        LOG_DEBUG("Image2D destroyed: %s", debugName ? debugName : "Unnamed");
    }

    deviceHandle = VK_NULL_HANDLE;
    stage = 0;
    access = 0;
    layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

bool Image2D::CopyBuffer(VkCommandBuffer cmd, VkBuffer srcBuffer, VkDeviceSize bufferOffset, VkOffset3D imageOffset)
{
    VK_CHECK_HANDLE(cmd, VkCommandBuffer);
    VK_CHECK_HANDLE(srcBuffer, VkBuffer);

    VkBufferImageCopy region{};
    region.bufferOffset = bufferOffset;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = aspect;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = imageOffset;
    region.imageExtent = { extent.width, extent.height, 1 };
    vkCmdCopyBufferToImage(cmd, srcBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    return true;
}

bool Image2D::CopyImage(VkCommandBuffer cmd, VkImage srcImage, VkExtent2D copyExtent, VkImageAspectFlags srcAspect)
{
    VK_CHECK_HANDLE(cmd, VkCommandBuffer);
    VK_CHECK_HANDLE(srcImage, VkImage);
    
    VkImageCopy copyRegion{};
    copyRegion.srcSubresource.aspectMask = srcAspect;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource.aspectMask = aspect;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstOffset = {0, 0, 0};
    copyRegion.extent = { copyExtent.width, copyExtent.height, 1 };
    vkCmdCopyImage(cmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    return true;
}

void Image2D::Transition(VkCommandBuffer cmd, VkImageLayout dstLayout, VkPipelineStageFlags dstStage, VkAccessFlags dstAccess, VkImageAspectFlags aspectFlags, uint32_t mipCount)
{
    VkPipelineStageFlags srcStage;
    VkAccessFlags srcAccess;
    switch (layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED: srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; srcAccess = 0; break;
        case VK_IMAGE_LAYOUT_GENERAL: srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; srcAccess = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT; break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT; srcAccess = VK_ACCESS_TRANSFER_READ_BIT; break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT; srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT; break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT; srcAccess = VK_ACCESS_SHADER_READ_BIT; break;
        default: srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; srcAccess = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; break;
    }
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = layout;
    barrier.newLayout = dstLayout;
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = aspectFlags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipCount;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layers;
    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    layout = dstLayout;
    stage = dstStage;
    access = dstAccess;
}
