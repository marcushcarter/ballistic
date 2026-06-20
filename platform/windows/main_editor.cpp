#include <core/application/editor_application.h>
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    ballistic::ApplicationCreateInfo info;
    info.window_title = L"Ballistic Editor";
    info.width = 1280;
    info.height = 720;

    ballistic::EditorApplication app;
    app.create(info);
    return app.run();
}