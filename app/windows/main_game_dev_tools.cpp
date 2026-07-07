#include <game/game_application.h>
#include <windows.h>

int main() {
    ballistic::ApplicationCreateInfo info;
    info.window_title = "Ballistic Game (Dev Tools)";
    info.width = 1280;
    info.height = 720;

    ballistic::GameApplication app;
    app.create(info);
    return app.run();
}