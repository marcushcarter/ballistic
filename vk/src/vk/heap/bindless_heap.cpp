#include <vk/heap/bindless_heap.h>
// #include "graphics/vk/misc/utils.h"

bool BindlessHeap::Create(VkDevice device, const BindlessHeapDesc& d)
{
    sampledAlloc.cap = d.sampledImages;
    storageAlloc.cap = d.storageImages;
    samplerAlloc.cap = d.samplers;
    deviceHandle = device;

    VkDescriptorSetLayoutBinding bindings[3]{};
    bindings[BINDING_SAMPLED] = { BINDING_SAMPLED, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, d.sampledImages, VK_SHADER_STAGE_ALL, nullptr };
    bindings[BINDING_STORAGE] = { BINDING_STORAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, d.storageImages, VK_SHADER_STAGE_ALL, nullptr };
    bindings[BINDING_SAMPLER] = { BINDING_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER, d.samplers, VK_SHADER_STAGE_ALL, nullptr };

    const VkDescriptorBindingFlags bf = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    VkDescriptorBindingFlags flags[3]{ bf, bf, bf};

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
    flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsInfo.bindingCount = 3;
    flagsInfo.pBindingFlags = flags;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = &flagsInfo;
    layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    layoutInfo.bindingCount = 3;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
        // LOG_ERROR("BindlessHeap: vkCreateDescriptorSetLayout failed: (%s)", d.debugName ? d.debugName : "unnamed");
        return false;
    }

    VkDescriptorPoolSize poolSizes[3] = {
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, d.sampledImages },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, d.storageImages },
        { VK_DESCRIPTOR_TYPE_SAMPLER, d.samplers },
    };
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 3;
    poolInfo.pPoolSizes = poolSizes;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        // LOG_ERROR("BindlessHeap: vkCreateDescriptorPool failed: (%s)", d.debugName ? d.debugName : "unnamed");
        return false;
    }

    VkDescriptorSetAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc.descriptorPool = pool;
    alloc.descriptorSetCount = 1;
    alloc.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(device, &alloc, &set) != VK_SUCCESS) {
        // LOG_ERROR("BindlessHeap: vkAllocateDescriptorSets failed: (%s)", d.debugName ? d.debugName : "unnamed");
        return false;
    }

    // SetObjectName(device, VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)set, d.debugName);
    // LOG_DEBUG("BindlessHeap created (%u sampled, %u storage, %u samplers)", d.sampledImages, d.storageImages, d.samplers);
    return true;
}

void BindlessHeap::Destroy()
{
    if (pool) { vkDestroyDescriptorPool(deviceHandle, pool, nullptr); pool = VK_NULL_HANDLE; }
    if (layout) { vkDestroyDescriptorSetLayout(deviceHandle, layout, nullptr); layout = VK_NULL_HANDLE; }
    set = VK_NULL_HANDLE;
}

static void WriteImage(VkDevice device, VkDescriptorSet set, uint32_t binding, uint32_t index, VkDescriptorType type, VkImageView view, VkImageLayout layout)
{
    VkDescriptorImageInfo info{};
    info.imageView = view;
    info.imageLayout = layout;

    VkWriteDescriptorSet w{};
    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w.dstSet = set;
    w.dstBinding = binding;
    w.dstArrayElement = index;
    w.descriptorCount = 1;
    w.descriptorType = type;
    w.pImageInfo = &info;

    vkUpdateDescriptorSets(device, 1, &w, 0, nullptr);
}

uint32_t BindlessHeap::RegisterSampledImage(VkImageView view)
{
    uint32_t i = sampledAlloc.Acquire();
    WriteImage(deviceHandle, set, BINDING_SAMPLED, i, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    return i;
}

uint32_t BindlessHeap::RegisterStorageImage(VkImageView view)
{
    uint32_t i = storageAlloc.Acquire();
    WriteImage(deviceHandle, set, BINDING_STORAGE, i, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, view, VK_IMAGE_LAYOUT_GENERAL);
    return i;
}

uint32_t BindlessHeap::RegisterSampler(VkSampler sampler)
{
    uint32_t i = samplerAlloc.Acquire();
    VkDescriptorImageInfo info{}; info.sampler = sampler;
    VkWriteDescriptorSet w{};
    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w.dstSet = set; w.dstBinding = BINDING_SAMPLER; w.dstArrayElement = i;
    w.descriptorCount = 1; w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    w.pImageInfo = &info;
    vkUpdateDescriptorSets(deviceHandle, 1, &w, 0, nullptr);
    return i;
}
