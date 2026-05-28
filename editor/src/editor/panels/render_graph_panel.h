#pragma once
#include "pch.h"

struct EditorContext;

struct RenderGraphPanel
{
    ImNodesEditorContext* nodesContext = nullptr;

    void OnProjectOpened(const std::filesystem::path& path);
    void OnProjectClosed(const std::filesystem::path& path);
    void Save(const std::filesystem::path& path);

    void Draw(EditorContext& ctx);
};