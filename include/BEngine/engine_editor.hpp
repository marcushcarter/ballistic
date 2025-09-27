#pragma once

#include "BEngine/engine.hpp"

#include "BEngine/imgui/imgui.h"
#include "BEngine/imgui/imgui_internal.h"
#include "BEngine/imgui/imgui_impl_glfw.h"
#include "BEngine/imgui/imgui_impl_opengl3.h"
#include "BEngine/imgui/implot.h"

#include "BEngine/imgui/ImGuiFileDialog.h"

namespace BE {

class Editor {
public:
    Anchor selectedAnchor = -1;

    Editor(Engine* enginePtr);
    ~Editor();

    void beginFrame();
    void showPanels();
    void endFrame();
    
    void Frame();

    void Menu();
    void Viewport();
    void Heirarchy();
    void Popups();
    void Resources();
    void Inspector();

private:
    Engine* engine;

    Framebuffer meshPreviewFB;
};

}; // BE namespace