#pragma once
#include <functional>
#include <filesystem>

namespace ballistic {

namespace drivers { struct WindowDriverWin32; }
namespace drivers { struct ImGuiDriver; }
struct Renderer;
struct Project;
struct EditorRenderPath;
struct EditorSettings;
struct PopupManager;

struct EditorContext
{
    drivers::WindowDriverWin32* win32 = nullptr;
    drivers::ImGuiDriver* imgui = nullptr;
    Renderer* renderer = nullptr;
    EditorRenderPath* render_path = nullptr;
    Project* project = nullptr;
    EditorSettings* settings = nullptr;
    PopupManager* popups = nullptr;
    
    std::function<void(const std::filesystem::path&)> open_project_callback;
    std::function<void()> close_project_callback;
};

}