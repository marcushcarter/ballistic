#include "PipelineCache.h"

bool PipelineCache::Load(VkDevice device, const std::string& path)
{
    VK_CHECK_HANDLE(device, VkDevice);

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
            LOG_WARN("Failed to read pipeline cache, creating empty cache");
            data.clear();
        }
    }

    VkPipelineCacheCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    createInfo.initialDataSize = data.size();
    createInfo.pInitialData = data.empty() ? nullptr : data.data();

    if (vkCreatePipelineCache(device, &createInfo, nullptr, &cache) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan pipeline cache");
        return false;
    }

    LOG_DEBUG("Pipeline Cache loaded");
    return true;
}

bool PipelineCache::Save(VkDevice device, const std::string& path)
{
    VK_CHECK_HANDLE(device, VkDevice);
    VK_CHECK_HANDLE(cache, VkPipelineCache);

    size_t size = 0;
    vkGetPipelineCacheData(device, cache, &size, nullptr);
    if (size == 0) return false;

    std::vector<char> data(size);
    vkGetPipelineCacheData(device, cache, &size, data.data());

    std::filesystem::path fsPath(path);
    std::filesystem::create_directories(fsPath.parent_path());

    std::ofstream file(fsPath, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open pipeline cache for writing: %s", path.c_str());
        return false;
    }

    file.write(data.data(), data.size());
    LOG_DEBUG("Pipeline Cache saved");
    return file.good();
}

void PipelineCache::Destroy()
{
    if (cache != VK_NULL_HANDLE) {
        vkDestroyPipelineCache(deviceHandle, cache, nullptr);
        cache = VK_NULL_HANDLE;
        LOG_DEBUG("Pipeline Cache destroyed");
    }
    deviceHandle = VK_NULL_HANDLE;
}
