#pragma once
#include "pch.h"
#include "editor_context.h"
#include "panels/viewport_panel.h"
#include "panels/render_graph_panel.h"

struct Project;
struct Renderer;

struct Editor
{
    ViewportPanel viewport;
    RenderGraphPanel graph;

    std::string activeIniPath;

    float autosaveTimer = 0.0f;

    void OpenProject(const std::filesystem::path& path);
    void CloseProject(const std::filesystem::path& path);
    void SaveLayout(const std::filesystem::path& path);
    void SaveProjectAndLayout(EditorContext& ctx);

    void Update(EditorContext& ctx);
    void Draw(EditorContext& ctx);

    void DrawProjectPanel(EditorContext& ctx);
};