#pragma once

namespace ballistic {

struct Renderer;
struct EditorRenderPath;
struct DevTools;
namespace drivers { struct ImGuiDriver; }

struct EditorContext
{
    Renderer* renderer = nullptr;
    drivers::ImGuiDriver* imgui = nullptr;
    DevTools* dev = nullptr;
    EditorRenderPath* render_path = nullptr;
};

}