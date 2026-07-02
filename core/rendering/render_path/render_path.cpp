#include <core/rendering/render_path/render_path.h>

namespace ballistic {

Error RenderPath::create_resources()
{
    using enum Error;
    // base: create shared features. e.g. gbuffer.create(*device_driver), hbao.create(...)
    // none yet — empty for the current slice.
    return Ok;
}

void RenderPath::destroy_resources()
{
    // base: destroy shared features in reverse.
}

void RenderPath::build(RenderGraph& g)
{
    // shared deferred chain (none yet):
    // gbuffer.add(g); hbao.add(g); lighting.add(g); ...
    build_present(g);
}

}