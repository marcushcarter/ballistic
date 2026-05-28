#pragma once
#include "pch.h"
#include <core/application.h>
#include "editor/project_manager.h"
#include "editor/editor.h"
#include "editor/app_data.h"
#include "graphics/imgui_layer.h"

struct EditorApplication : Application
{
    AppDataPaths appData;

    ImGuiLayer imguiLayer;
    
    bool inProjectManager = true;
    bool pendingCloseProject = false;
    
    VkDescriptorSet finalTextureID = VK_NULL_HANDLE;
    VkDescriptorSet logoTextureID = VK_NULL_HANDLE;
    VkDescriptorSet logoLongTextureID = VK_NULL_HANDLE;

    ProjectManager projectManager;
    Editor editor;
    
    void OnInit() override;
    void OnUpdate() override;
    void OnShutdown() override;
    
    void OnProjectOpened(const std::filesystem::path& path) override;
    void OnProjectClosed() override;
};