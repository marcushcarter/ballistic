#pragma once
#include "pch.h"

struct Project;
struct Renderer;
struct EditorWorkspace;

struct EditorContext
{
    Project& project;
    Renderer& renderer;
    EditorWorkspace& workspace;
 
    VkDescriptorSet finalTextureID = VK_NULL_HANDLE;
 
    bool requestCloseProject = false;
};