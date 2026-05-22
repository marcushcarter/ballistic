#pragma once
#include "pch.h"

struct Instance
{
    VkInstance instance = VK_NULL_HANDLE;

    bool Create(const std::string& name = "My Vulkan App", uint32_t major = 1, uint32_t minor = 0, uint32_t patch = 0, const std::vector<const char*>& requiredExtensions = {}, const std::vector<const char*>& requiredLayers = {}, bool enableValidation = true);
    void Destroy();

    VkInstance Get() const { return instance; }

private:
    bool CheckLayerSupport(const std::vector<const char*>& layers);
    bool CheckExtensionSupport(const std::vector<const char*>& extensions);
};
