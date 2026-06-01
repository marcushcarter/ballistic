#pragma once
#include "pch.h"
#include "editor/editor_context.h"

struct RenderGraphPanel
{
    ImNodesEditorContext* context = nullptr;

    void Draw(EditorContext& ctx);
};
