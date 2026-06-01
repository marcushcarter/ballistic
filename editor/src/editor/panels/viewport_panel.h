#pragma once
#include "pch.h"
#include "editor/editor_context.h"

struct ViewportPanel
{
    uint32_t lastViewportW = 0, lastViewportH = 0;
    uint32_t pendingViewportW = 0, pendingViewportH = 0;
    bool viewportSizeChanged = false;

    void Draw(EditorContext& ctx);
};
