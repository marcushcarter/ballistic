#include <game/game_application.h>
#include <graphics/render_paths/game_render_path.h>
#include <core/log.h>

void GameApplication::OnInit()
{
    window.onFramebufferResize = [this](uint32_t w, uint32_t h) {
        renderer.RequestWindowResize(w, h);
        renderer.RequestSceneResize(w, h);
    };

    onProjectLoadFailed = [this]() {
        Destroy();
    };

    renderer.SetRenderPath(std::make_unique<GameRenderPath>(renderer));
    
    // OpenProject(std::filesystem::current_path());
    OpenProject("D:/Ballistic Games/ballistic-engine/docs/samples/Test_Project");
    
    LOG_DEBUG("Game initialized");
}

void GameApplication::OnUpdate()
{

}

void GameApplication::OnShutdown()
{
    LOG_DEBUG("Game shutdown");
}

void GameApplication::OnProjectOpened(const std::filesystem::path& path)
{
    (void)path;
    window.SetTitle(project.name.c_str());
    window.Show();
    // window.SetFullscreen(true);
}

void GameApplication::OnProjectClosed()
{

}
