#pragma once
#include "pch.h"

struct EditorContext;

struct ViewportPanel
{
    uint32_t lastViewportW = 0, lastViewportH = 0;
    uint32_t pendingViewportW = 0, pendingViewportH = 0;
    bool viewportSizeChanged = false;

    void Draw(EditorContext& ctx);
};
