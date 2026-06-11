#include <descriptor/descriptor_set_layout.h>
#include <misc/utils.h>

namespace Ballistic::Vulkan
{
bool DescriptorSetLayout::Create(VkDevice device, const DescriptorSetLayoutDesc& d)
{
    Destroy();
    debugName = d.debugName;
    deviceHandle = device;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
    flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsInfo.bindingCount = (uint32_t)d.bindingFlags.size();
    flagsInfo.pBindingFlags = d.bindingFlags.data();

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = d.bindingFlags.empty() ? nullptr : &flagsInfo;
    createInfo.flags = d.layoutFlags;
    createInfo.bindingCount = static_cast<uint32_t>(d.bindings.size());
    createInfo.pBindings = d.bindings.data();

    if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layout) != VK_SUCCESS) {
        // LOG_ERROR("Descriptor Set Layout create failed: %s - vkCreateDescriptorSetLayout", debugName ? debugName : "Unnamed");
        return false;
    }
    
    SetObjectName(device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)layout, d.debugName);
    // LOG_DEBUG("Descriptor Set Layout created: %s", debugName ? debugName : "Unnamed");
    return true;
}

void DescriptorSetLayout::Destroy()
{
    if (layout) {
        vkDestroyDescriptorSetLayout(deviceHandle, layout, nullptr);
        layout = VK_NULL_HANDLE;
        // LOG_DEBUG("Descriptor Set Layout destroyed: %s", debugName ? debugName : "Unnamed");
    }
}
}