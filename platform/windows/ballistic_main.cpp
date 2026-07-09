#include <windows.h>
#include <core/application/application.h>
#include <cstdio>

#if defined(BALLISTIC_EDITOR)
    #include <editor/editor_application.h>
    #include <core/io/path.h>
    #include <filesystem>
#elif defined(BALLISTIC_GAME)
    #include <game/game_application.h>
#else
    #error "ballistic_main.cpp requires BALLISTIC_EDITOR or BALLISTIC_GAME"
#endif

#if !defined(BALLISTIC_CONSOLE)
static void attach_console_or_null()
{
    FILE* dummy;
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
        freopen_s(&dummy, "CONIN$",  "r", stdin);
    } else {
        freopen_s(&dummy, "NUL", "w", stdout);
        freopen_s(&dummy, "NUL", "w", stderr);
        freopen_s(&dummy, "NUL", "r", stdin);
    }
}
#endif

static std::unique_ptr<ballistic::Application> create_application(
    ballistic::ApplicationCreateInfo& info, [[maybe_unused]] std::string& ini_storage)
{
#if defined(BALLISTIC_EDITOR)
    info.window_title = "Ballistic Editor";
    ini_storage = (ballistic::Paths::roaming_data() / "editor_layout.cfg").string();
    info.ini_path = ini_storage.c_str();
    return std::make_unique<ballistic::EditorApplication>();

#elif defined(BALLISTIC_GAME)
  #if defined(BALLISTIC_DEV_TOOLS)
    info.window_title = "Ballistic Game (Dev Tools)";
  #else
    info.window_title = "Ballistic Game";
  #endif
    return std::make_unique<ballistic::GameApplication>();
#endif
}

static int run_app()
{
    ballistic::ApplicationCreateInfo info;
    info.width  = 1280;
    info.height = 720;

    std::string ini_storage;
    auto app = create_application(info, ini_storage);

    app->create(info);
    return app->run();
}

#if defined(BALLISTIC_CONSOLE)
int main()
{
    return run_app();
}
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    attach_console_or_null();
    return run_app();
}
#endif