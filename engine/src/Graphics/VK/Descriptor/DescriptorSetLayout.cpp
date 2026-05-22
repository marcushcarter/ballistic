#include "DescriptorSetLayout.h"

bool DescriptorSetLayout::Create(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& bindings, const std::vector<VkDescriptorBindingFlags>& bindingFlags, VkDescriptorSetLayoutCreateFlags layoutFlags)
{
    // VK_CHECK_HANDLE(device, VkDevice, false);

    Destroy();
    deviceHandle = device;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
    flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsInfo.bindingCount = (uint32_t)bindingFlags.size();
    flagsInfo.pBindingFlags = bindingFlags.data();

    VkDescriptorSetLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = (uint32_t)bindings.size();
    info.pBindings = bindings.data();
    info.flags = layoutFlags;
    if (!bindingFlags.empty()) info.pNext = &flagsInfo;

    if (vkCreateDescriptorSetLayout(device, &info, nullptr, &layout) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan descriptor set layout");
        return false;
    }

    LOG_DEBUG("Descriptor Set Layout created");
    return true;
}

void DescriptorSetLayout::Destroy()
{
    if (layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(deviceHandle, layout, nullptr);
        layout = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Descriptor Set Layout destroyed");
    }
}
