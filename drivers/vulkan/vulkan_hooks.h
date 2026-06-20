// basically a class with virtual functions to do things like create vulkan instance and get physical device etc etc

// SINGLETON

// includes ballistic_vulkan

#pragma once

class VulkanHooks {
private:
    static VulkanHooks* singleton;

public:
    VulkanHooks();
    virtual ~VulkanHooks();
    virtual bool create_vulkan_instance() {}
};
