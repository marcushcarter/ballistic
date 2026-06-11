#include <descriptor/descriptor_pool.h>
#include <misc/utils.h>
#include <vector>

namespace Ballistic::Vulkan
{
bool DescriptorPool::Create(VkDevice device, const DescriptorPoolDesc& d)
{    
    Destroy();
    debugName = d.debugName;
    deviceHandle = device;
    
    std::vector<VkDescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    auto add = [&](VkDescriptorType type, uint32_t count) {
        if (count > 0) {
            poolSizes.push_back({ type, count });
            maxSets += count;
        }
    };

    add(VK_DESCRIPTOR_TYPE_SAMPLER, d.samplers);
    add(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, d.combinedImageSamplers);
    add(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, d.sampledImages);
    add(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, d.storageImages);
    add(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, d.uniformTexelBuffers);
    add(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, d.storageTexelBuffers);
    add(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, d.uniformBuffers);
    add(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, d.storageBuffers);
    add(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, d.uniformBuffersDynamic);
    add(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, d.storageBuffersDynamic);
    add(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, d.inputAttachments);

    if (poolSizes.empty()) {
        // LOG_ERROR("Descriptor Pool create failed: %s - no pool sizes specified", debugName ? debugName : "Unnamed");
        return false;
    }

    VkDescriptorPoolCreateFlags flags = 0;
    if (d.freeable)  flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    if (d.afterBind) flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.flags = flags;
    createInfo.maxSets = maxSets;
    createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    createInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(deviceHandle, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        // LOG_ERROR("Descriptor Pool create failed: %s - vkCreateDescriptorPool", debugName ? debugName : "Unnamed");
        return false;
    }

    SetObjectName(device, VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)descriptorPool, d.debugName);
    // LOG_DEBUG("Descriptor Pool created: %s (%u max sets)", debugName ? debugName : "Unnamed", maxSets);
    return true;
}

void DescriptorPool::Destroy()
{
    if (descriptorPool) {
        vkDestroyDescriptorPool(deviceHandle, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
        // LOG_DEBUG("Descriptor Pool destroyed: %s", debugName ? debugName : "Unnamed");
    }
}
}