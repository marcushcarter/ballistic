#pragma once
#include "pch.h"
#include <core/application.h>

struct EditorApplication : Application
{
    VkDescriptorSet finalTextureID = VK_NULL_HANDLE;
    VkDescriptorSet logoTextureID = VK_NULL_HANDLE;
    VkDescriptorSet logoLongTextureID = VK_NULL_HANDLE;

    std::filesystem::path roamingRoot;
    std::filesystem::path localRoot;
    
    void OnInit() override;
    void OnUpdate() override;
    void OnShutdown() override;

    void SetupAppData(std::string& outIniPath);
};