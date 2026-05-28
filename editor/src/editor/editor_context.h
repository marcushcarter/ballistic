#pragma once
#include "pch.h"

struct Project;
struct Renderer;
struct SplashRenderer;

struct EditorContext
{
    Project& project;
    Renderer& renderer;
    SplashRenderer& splash;

    VkDescriptorSet finalTextureID = VK_NULL_HANDLE;

    bool requestCloseProject = false;
    bool requestSave = false;
};