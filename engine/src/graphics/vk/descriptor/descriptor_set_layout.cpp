#include "descriptor_set_layout.h"
#include "graphics/vk/misc/utils.h"

bool DescriptorSetLayout::Create(VkDevice device, const DescriptorSetLayoutDesc& desc)
{
    VK_CHECK_HANDLE(device, VkDevice);

    Destroy();
    debugName = desc.debugName;
    deviceHandle = device;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
    flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsInfo.bindingCount = (uint32_t)desc.bindingFlags.size();
    flagsInfo.pBindingFlags = desc.bindingFlags.data();

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = desc.bindingFlags.empty() ? nullptr : &flagsInfo;
    createInfo.flags = desc.layoutFlags;
    createInfo.bindingCount = static_cast<uint32_t>(desc.bindings.size());
    createInfo.pBindings = desc.bindings.data();

    if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layout) != VK_SUCCESS) {
        LOG_ERROR("Descriptor Set Layout create failed: %s - vkCreateDescriptorSetLayout", debugName ? debugName : "Unnamed");
        return false;
    }
    
    SetObjectName(device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)layout, debugName);
    LOG_DEBUG("Descriptor Set Layout created: %s", debugName ? debugName : "Unnamed");
    return true;
}

void DescriptorSetLayout::Destroy()
{
    if (layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(deviceHandle, layout, nullptr);
        layout = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Descriptor Set Layout destroyed: %s", debugName ? debugName : "Unnamed");
    }
}
