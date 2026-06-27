#include <core/application/editor_application.h>
#include <windows.h>
#include <cstdio>

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

    ballistic::ApplicationCreateInfo info;
    info.window_title = "Ballistic Editor";
    info.width = 1280;
    info.height = 720;

    ballistic::EditorApplication app;
    app.create(info);
    return app.run();
}