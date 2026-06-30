#include <core/rendering/game_render_path.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/log/log.h>

namespace ballistic {

Error GameRenderPath::create_resources()
{
    using enum Error;

    Error err = RenderPath::create_resources();
    if (err != Ok) return err;

    return Ok;
}

void GameRenderPath::destroy_resources()
{
    RenderPath::destroy_resources();
}

void GameRenderPath::build_present(RenderGraph& g)
{
    (void)g;
    log_write("GAME RENDER PATH");
}

}