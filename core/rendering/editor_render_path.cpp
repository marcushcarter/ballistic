#include <core/rendering/editor_render_path.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/log/log.h>

namespace ballistic {

Error EditorRenderPath::create_resources()
{
    using enum Error;

    Error err = RenderPath::create_resources();
    if (err != Ok) return err;

    return Ok; 
}

void EditorRenderPath::destroy_resources()
{
    RenderPath::destroy_resources();
}

void EditorRenderPath::build_present(RenderGraph& g)
{
    (void)g;
    log_write("EDITOR RENDER PATH");
}

}