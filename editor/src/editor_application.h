#pragma once
#include "pch.h"
#include <core/application.h>
#include "project/project_manager.h"

struct EditorApplication : Application
{
    std::filesystem::path roamingRoot;
    std::filesystem::path localRoot;
    
    bool inProjectManager = true;

    std::string activeIniPath;
    
    VkDescriptorSet finalTextureID = VK_NULL_HANDLE;
    VkDescriptorSet logoTextureID = VK_NULL_HANDLE;
    VkDescriptorSet logoLongTextureID = VK_NULL_HANDLE;

    bool pendingCloseProject = false;
    ProjectManager projectManager;

    char addImageNameBuffer[256] = {};
    int addImageFormatIndex = 0;
    int addImageSizeModeIndex = 0;
    float addImageRelW = 1.0f;
    float addImageRelH = 1.0f;
    uint32_t addImageFixedW = 1920;
    uint32_t addImageFixedH = 1080;

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
    
    void OnProjectOpened(const std::filesystem::path& path) override;
    void OnProjectClosed() override;

    void SetupAppData();
    void DrawEditor();
};