#pragma once

namespace ballistic {

namespace drivers { struct WindowDriverWin32; }
namespace drivers { struct ImGuiDriver; }
struct Renderer;
struct Project;
struct EditorRenderPath;
struct EditorSettings;

struct EditorContext
{
    drivers::WindowDriverWin32* win32 = nullptr;
    drivers::ImGuiDriver* imgui = nullptr;
    Renderer* renderer = nullptr;
    EditorRenderPath* render_path = nullptr;
    Project* project = nullptr;
    EditorSettings* settings = nullptr;
};

}