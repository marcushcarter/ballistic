#include "game_application.h"

int Main()
{
    GameApplication app;
    app.Create("Ballistic Game", 1280, 720);
    app.Run();
    app.Destroy();
    return 0;
}

#ifdef _WIN32
#ifndef BALLISTIC_CONSOLE
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    return Main();
}

#endif
#endif

int main()
{
    return Main();
}