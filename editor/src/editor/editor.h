#pragma once
#include "pch.h"
#include "editor_context.h"

struct Project;
struct Renderer;

struct Editor
{
    std::string activeIniPath;
    ImNodesEditorContext* renderGraphContext = nullptr;

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
    void SaveLayout(const std::filesystem::path& path);
    void SaveProjectAndLayout(EditorContext& ctx);

    void Update(EditorContext& ctx);
    void Draw(EditorContext& ctx);

    void DrawDockSpace();
    void DrawViewport(EditorContext& ctx);
    void DrawProjectPanel(EditorContext& ctx);
    void DrawRenderGraphPanel();
};