#pragma once
#include "pch.h"

struct Project;
struct Renderer;

struct Editor
{
    std::string activeIniPath;
    ImNodesEditorContext* renderGraphContext = nullptr;

    bool autosaveEnabled = true;
    float autosaveInterval = 120.0f;
    float autosaveTimer = 0.0f;

    char addImageNameBuffer[256] = {};
    int  addImageFormatIndex = 0;
    int  addImageSizeModeIndex = 0;
    float addImageRelW = 1.0f;
    float addImageRelH = 1.0f;
    uint32_t addImageFixedW = 1920;
    uint32_t addImageFixedH = 1080;

    uint32_t lastViewportW = 0;
    uint32_t lastViewportH = 0;
    uint32_t pendingViewportW = 0;
    uint32_t pendingViewportH = 0;
    bool viewportSizeChanged = false;

    void OpenProject(const std::filesystem::path& path);
    void CloseProject(const std::filesystem::path& path);
    void Save(const std::filesystem::path& path);

    void Update(Project& project);
    void Draw(Project& project, Renderer& renderer, bool& pendingCloseProject, VkDescriptorSet finalTextureID);

    void DrawDockSpace();
    void DrawViewport(Renderer& renderer, VkDescriptorSet finalTextureID);
    void DrawProjectPanel(Project& project, Renderer& renderer, bool& pendingCloseProject);
    void DrawRenderGraphPanel();
};