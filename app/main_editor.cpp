#include <editor/editor_application.h>
#include <core/io/path.h>
#include <windows.h>
#include <cstdio>
#include <shlobj.h>
#include <filesystem>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    bool has_console = AttachConsole(ATTACH_PARENT_PROCESS) != 0;
    
    FILE* dummy;
    if (has_console) {
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
        freopen_s(&dummy, "CONIN$", "r", stdin);
    } else {
        freopen_s(&dummy, "NUL", "w", stdout);
        freopen_s(&dummy, "NUL", "w", stderr);
        freopen_s(&dummy, "NUL", "r", stdin);
    }
    
    std::filesystem::path ini_path = ballistic::Paths::roaming_data() / "editor_layout.cfg";
    std::string ini_path_str = ini_path.string();

    ballistic::ApplicationCreateInfo info;
    info.window_title = "Ballistic Editor";
    info.width = 1280;
    info.height = 720;
    info.ini_path = ini_path_str.c_str();

    ballistic::EditorApplication app;
    app.create(info);
    return app.run();
}