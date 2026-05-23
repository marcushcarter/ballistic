#pragma once
#include "pch.h"
#include <core/application.h>

struct EditorApplication : Application
{
    VkDescriptorSet finalTextureID = VK_NULL_HANDLE;
    
    void OnInit() override;
    void OnUpdate() override;
    void OnShutdown() override;
};