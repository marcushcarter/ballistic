#pragma once
#include "pch.h"

struct EditorContext;

struct ViewportPanel
{
    uint32_t lastViewportW = 0;
    uint32_t lastViewportH = 0;
    uint32_t pendingViewportW = 0;
    uint32_t pendingViewportH = 0;
    bool viewportSizeChanged = false;

    void Draw(EditorContext& ctx);
};