#include "transient_resource.h"

static void SetObjectName(VkDevice device, VkObjectType type, uint64_t handle, const char* name)
{
    if (!name) return;
    auto fn = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
    if (!fn) return;
    VkDebugUtilsObjectNameInfoEXT info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
    info.objectType   = type;
    info.objectHandle = handle;
    info.pObjectName  = name;
    fn(device, &info);
}

bool PhysicalImage::CreateUnbound(VkDevice device, const TransientImageDesc& d, VkExtent2D resolved)
{
    VK_CHECK_HANDLE(device, VkDevice);

    extent = resolved;
    format = d.format;
    aspect = d.aspect;
    mipLevels = d.mipLevels;
    layers = d.layers;

    VkImageCreateInfo ci{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    ci.flags = VK_IMAGE_CREATE_ALIAS_BIT;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = d.format;
    ci.extent = { resolved.width, resolved.height, 1 };
    ci.mipLevels = d.mipLevels;
    ci.arrayLayers = d.layers;
    ci.samples = d.samples;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = d.usage;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    if (vkCreateImage(device, &ci, nullptr, &image) != VK_SUCCESS) {
        LOG_ERROR("PhysicalImage: vkCreateImage failed: (%s)", d.debugName ? d.debugName : "unnamed");
        return false;
    }

    vkGetImageMemoryRequirements(device, image, &memReq);
    SetObjectName(device, VK_OBJECT_TYPE_IMAGE, (uint64_t)image, d.debugName);
    return true;
}

bool PhysicalImage::BindAndView(VkDevice device, VmaAllocator vma, VmaAllocation backing, VkDeviceSize offset)
{    
    if (vmaBindImageMemory2(vma, backing, offset, image, nullptr) != VK_SUCCESS) {
        LOG_ERROR("PhysicalImage: vmaBindImageMemory2 failed (offset %llu)", (unsigned long long)offset);
        return false;
    }

    VkImageViewCreateInfo vi{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    vi.image = image;
    vi.viewType = (layers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    vi.format = format;
    vi.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    vi.subresourceRange = { aspect, 0, mipLevels, 0, layers };

    if (vkCreateImageView(device, &vi, nullptr, &view) != VK_SUCCESS) {
        LOG_ERROR("PhysicalImage: vkCreateImageView failed");
        return false;
    }
    return true;
}

void PhysicalImage::Destroy(VkDevice device)
{
    if (view != VK_NULL_HANDLE) { vkDestroyImageView(device, view, nullptr); view  = VK_NULL_HANDLE; }
    if (image != VK_NULL_HANDLE) { vkDestroyImage(device, image, nullptr); image = VK_NULL_HANDLE; }
}

bool PhysicalBuffer::CreateUnbound(VkDevice device, const TransientBufferDesc& d)
{
    VK_CHECK_HANDLE(device, VkDevice);

    if (d.size == 0) {
        LOG_ERROR("PhysicalBuffer: zero-size transient (%s)", d.debugName ? d.debugName : "unnamed");
        return false;
    }
    size = d.size;

    VkBufferCreateInfo ci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    ci.size = d.size;
    ci.usage = d.usage;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &ci, nullptr, &buffer) != VK_SUCCESS) {
        LOG_ERROR("PhysicalBuffer: vkCreateBuffer failed (%s)", d.debugName ? d.debugName : "unnamed");
        return false;
    }

    vkGetBufferMemoryRequirements(device, buffer, &memReq);
    SetObjectName(device, VK_OBJECT_TYPE_BUFFER, (uint64_t)buffer, d.debugName);
    return true;
}

bool PhysicalBuffer::Bind(VmaAllocator vma, VmaAllocation backing, VkDeviceSize offset)
{    
    if (vmaBindBufferMemory2(vma, backing, offset, buffer, nullptr) != VK_SUCCESS) {
        LOG_ERROR("PhysicalBuffer: vmaBindBufferMemory2 failed (offset %llu)", (unsigned long long)offset);
        return false;
    }
    return true;
}

void PhysicalBuffer::Destroy(VkDevice device)
{
    if (buffer != VK_NULL_HANDLE) { vkDestroyBuffer(device, buffer, nullptr); buffer = VK_NULL_HANDLE; }
}
