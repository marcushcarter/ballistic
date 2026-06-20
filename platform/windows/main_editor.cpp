#include <core/application/editor_application.h>
#include <windows.h>
#include <cstdio>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    AttachConsole(ATTACH_PARENT_PROCESS);

    FILE* dummy;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    freopen_s(&dummy, "CONIN$", "r", stdin);

    ballistic::ApplicationCreateInfo info;
    info.window_title = L"Ballistic Editor";
    info.width = 1280;
    info.height = 720;

    ballistic::EditorApplication app;
    app.create(info);
    return app.run();
}