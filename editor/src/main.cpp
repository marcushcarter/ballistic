#include "editor_app.h"

#ifdef _WIN32
#ifndef BALLISTIC_CONSOLE
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    return editor_main();
}

#endif
#endif

int main()
{
    return editor_main();
}