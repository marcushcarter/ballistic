#pragma once
#include "pch.h"

struct EditorContext;

struct RenderGraphPanel
{
    ImNodesEditorContext* context = nullptr;

    void Draw(EditorContext& ctx);
};
