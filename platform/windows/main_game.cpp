#include <core/application/game_application.h>
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    ballistic::ApplicationCreateInfo info;
    info.window_title = L"Ballistic Game";
    info.width = 1280;
    info.height = 720;

    ballistic::GameApplication app;
    app.create(info);
    return app.run();
}