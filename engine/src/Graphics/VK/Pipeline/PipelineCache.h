#pragma once
#include "pch.h"

struct PipelineCache
{
    VkPipelineCache cache = VK_NULL_HANDLE;

    bool Load(VkDevice device, const std::string& path);
    bool Save(VkDevice device, const std::string& path);
    void Destroy();

    VkPipelineCache Get() const { return cache; }

private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};