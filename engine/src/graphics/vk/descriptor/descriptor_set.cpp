#include "descriptor_set.h"

bool DescriptorSet::Allocate(VkDevice device, const DescriptorSetDesc& desc)
{
    VK_CHECK_HANDLE(device, VkDevice);
    VK_CHECK_HANDLE(desc.setLayout, VkDescriptorSetLayout);

    debugName = desc.debugName;
    deviceHandle = device;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = desc.pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &desc.setLayout;

    if (vkAllocateDescriptorSets(device, &allocInfo, &set) != VK_SUCCESS) {
        LOG_ERROR("Descriptor Set create failed: %s - vkAllocateDescriptorSets", debugName ? debugName : "Unnamed");
        return false;
    }
    
    if (debugName) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
        nameInfo.objectHandle = (uint64_t)set;
        nameInfo.pObjectName = debugName;
        auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
        if (func) func(device, &nameInfo);
    }

    LOG_DEBUG("Descriptor Set allocated: %s", debugName ? debugName : "Unnamed");
    return true;
}

void DescriptorSet::SetBuffers(uint32_t binding, VkDescriptorType type, const std::vector<VkBuffer>& buffers, VkDeviceSize range, uint32_t dstArrayElement)
{
    std::vector<VkDescriptorBufferInfo> infos(buffers.size());
    for (uint32_t i = 0; i < buffers.size(); i++) {
        infos[i].buffer = buffers[i];
        infos[i].offset = 0;
        infos[i].range= range;
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.dstArrayElement = dstArrayElement;
    write.descriptorCount = (uint32_t)buffers.size();
    write.descriptorType = type;
    write.pBufferInfo = infos.data();

    vkUpdateDescriptorSets(deviceHandle, 1, &write, 0, nullptr);
}

void DescriptorSet::SetImages(uint32_t binding, const std::vector<VkImageView>& views, VkSampler sampler, VkImageLayout layout, uint32_t dstArrayElement)
{
    std::vector<VkDescriptorImageInfo> infos(views.size());
    for (uint32_t i = 0; i < views.size(); i++) {
        infos[i].imageView = views[i];
        infos[i].sampler = sampler;
        infos[i].imageLayout = layout;
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.dstArrayElement = dstArrayElement;
    write.descriptorCount = (uint32_t)views.size();
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = infos.data();

    vkUpdateDescriptorSets(deviceHandle, 1, &write, 0, nullptr);
}

void DescriptorSet::SetStorageImages(uint32_t binding, const std::vector<VkImageView>& views, VkImageLayout layout, uint32_t dstArrayElement)
{
    std::vector<VkDescriptorImageInfo> infos(views.size());
    for (uint32_t i = 0; i < views.size(); i++) {
        infos[i].imageView = views[i];
        infos[i].sampler = VK_NULL_HANDLE;
        infos[i].imageLayout = layout;
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.dstArrayElement = dstArrayElement;
    write.descriptorCount = (uint32_t)views.size();
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    write.pImageInfo = infos.data();

    vkUpdateDescriptorSets(deviceHandle, 1, &write, 0, nullptr);
}
