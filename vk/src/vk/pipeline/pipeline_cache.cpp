#include<vk/pipeline/pipeline_cache.h>
// #include "graphics/vk/misc/utils.h"
#include <fstream>

bool PipelineCache::Load(VkDevice device, const std::filesystem::path& path)
{
    Destroy();
    deviceHandle = device;

    std::filesystem::path fsPath(path);
    std::filesystem::create_directories(fsPath.parent_path());

    std::vector<char> data;
    std::ifstream file(fsPath, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        data.resize(file.tellg());
        file.seekg(0);
        if (!file.read(data.data(), data.size())) {
            // LOG_WARN("Failed to read pipeline cache, creating empty cache");
            data.clear();
        }
    }

    VkPipelineCacheCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    createInfo.initialDataSize = data.size();
    createInfo.pInitialData = data.empty() ? nullptr : data.data();

    if (vkCreatePipelineCache(device, &createInfo, nullptr, &cache) != VK_SUCCESS) {
        // LOG_ERROR("Failed to create Vulkan pipeline cache");
        return false;
    }

    // SetObjectName(device, VK_OBJECT_TYPE_PIPELINE_CACHE, (uint64_t)cache, "PipelineCache");
    // LOG_DEBUG("Pipeline Cache loaded");
    return true;
}

bool PipelineCache::Save(VkDevice device, const std::filesystem::path& path)
{
    size_t size = 0;
    vkGetPipelineCacheData(device, cache, &size, nullptr);
    if (size == 0) return false;

    std::vector<char> data(size);
    vkGetPipelineCacheData(device, cache, &size, data.data());

    std::filesystem::path fsPath(path);
    std::filesystem::create_directories(fsPath.parent_path());

    std::ofstream file(fsPath, std::ios::binary);
    if (!file.is_open()) {
        // LOG_ERROR("Failed to open pipeline cache for writing: %s", path.c_str());
        return false;
    }

    file.write(data.data(), data.size());
    // LOG_DEBUG("Pipeline Cache saved");
    return file.good();
}

void PipelineCache::Destroy()
{
    if (cache) {
        vkDestroyPipelineCache(deviceHandle, cache, nullptr);
        cache = VK_NULL_HANDLE;
        // LOG_DEBUG("Pipeline Cache destroyed");
    }
}
