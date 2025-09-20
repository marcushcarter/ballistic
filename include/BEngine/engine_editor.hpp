#pragma once

#include "BEngine/engine.hpp"

#include "BEngine/imgui/imgui.h"
#include "BEngine/imgui/imgui_impl_glfw.h"
#include "BEngine/imgui/imgui_impl_opengl3.h"
#include "BEngine/imgui/implot.h"

namespace BE {

class Editor {
public:
    Editor(Engine* enginePtr);
    ~Editor();

    void beginFrame();
    void showPanels();
    void showHeirarchy();
    void endFrame();

private:
    Engine* engine;
};

}; // BE namespace