#pragma once
#include "pch.h"
#include <core/application.h>
#include "Project/ProjectManager.h"

struct EditorApplication : Application
{
    std::filesystem::path roamingRoot;
    std::filesystem::path localRoot;
    
    bool inProjectManager = true;
    
    VkDescriptorSet finalTextureID = VK_NULL_HANDLE;
    VkDescriptorSet logoTextureID = VK_NULL_HANDLE;
    VkDescriptorSet logoLongTextureID = VK_NULL_HANDLE;

    ProjectManager projectManager;

    // struct Theme {
    //     ImVec4 background = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
    //     ImVec4 backgroundSecondary = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    //     ImVec4 backgroundTertiary = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    //     ImVec4 accent = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
    //     ImVec4 text = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    // } theme;
    
    void OnInit() override;
    void OnUpdate() override;
    void OnShutdown() override;

    void SetupAppData();
    void DrawEditor();    
};