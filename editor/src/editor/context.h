#pragma once
#include <pch.h>

struct Project;
struct Renderer;
struct EditorWorkspace;

struct EditorSettings;
struct EditorTheme;

struct EditorContext
{
    Project& project;
    Renderer& renderer;
    EditorWorkspace& workspace;

    EditorSettings& settings;
    EditorTheme& theme;

    VkDescriptorSet finalTextureID = VK_NULL_HANDLE;
 
    bool requestCloseProject = false;
};