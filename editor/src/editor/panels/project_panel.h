#pragma once
#include "pch.h"

struct EditorContext;

struct ProjectPanel
{
    // Add Image popup state
    char addImageNameBuffer[256] = {};
    int  addImageFormatIndex = 0;
    int  addImageSizeModeIndex = 0;
    float addImageRelW = 1.0f;
    float addImageRelH = 1.0f;
    uint32_t addImageFixedW = 1920;
    uint32_t addImageFixedH = 1080;

    void Draw(EditorContext& ctx);

private:
    void DrawAddImagePopup(EditorContext& ctx);
};