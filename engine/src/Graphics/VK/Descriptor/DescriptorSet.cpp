#include "DescriptorSet.h"

bool DescriptorSet::Allocate(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout)
{
    VK_CHECK_HANDLE(device, VkDevice);
    VK_CHECK_HANDLE(descriptorPool, VkDescriptorPool);
    VK_CHECK_HANDLE(descriptorSetLayout, VkDescriptorSetLayout);

    deviceHandle = device;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(device, &allocInfo, &set) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate Vulkan descriptor set");
        return false;
    }

    LOG_DEBUG("Descriptor Set allocated");
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
    write.descriptorCount = 1;
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
    write.descriptorCount = 1;
    write.descriptorCount = (uint32_t)views.size();
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    write.pImageInfo = infos.data();

    vkUpdateDescriptorSets(deviceHandle, 1, &write, 0, nullptr);
}
