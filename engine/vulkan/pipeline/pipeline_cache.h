#pragma once
#include <vulkan/vulkan.h>
#include <filesystem>

namespace Ballistic::Vulkan
{
struct PipelineCache
{
    VkPipelineCache cache = VK_NULL_HANDLE;

    bool Load(VkDevice device, const std::filesystem::path& path);
    bool Save(VkDevice device, const std::filesystem::path& path);
    void Destroy();

    VkPipelineCache Get() const { return cache; }

private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};
}