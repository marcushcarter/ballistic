#include <game/game_application.h>
#include <core/io/path.h>
#include <imgui.h>
#include <windows.h>
#include <shellapi.h>

namespace ballistic {

Error GameApplication::on_init()
{
    using enum Error;
    
    // if (project.load(Paths::executable_dir()) != Error::Ok) log_write("GameApplication: no project at exe root; running without content.");
    if (project.load("D:/TestBallistic") != Error::Ok) log_write("GameApplication: no project at exe root; running without content.");
    
    window_driver.window_set_title(window, project.name);
    
    return Ok;
}

void GameApplication::on_update(float p_dt)
{
    (void)p_dt;
    renderer.request_size(window.width, window.height);
}

void GameApplication::on_shutdown() {}

}