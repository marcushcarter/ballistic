#include <core/application/Application.h>
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Application::CreateInfo info;
    // info.windowTitle = L"Ballistic Game";
    // info.width = 1280;
    // info.height = 720;

    Application app(info);
    return app.Run();
}