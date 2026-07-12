#include <core/rendering/render_path/render_path.h>
#include <core/log/log.h>

namespace ballistic {

Error RenderPath::create_resources()
{
    using enum Error;
    
    temp.create_resources();

    return Ok;
}

void RenderPath::destroy_resources()
{
    
}

void RenderPath::build(RenderGraph& g)
{
    temp.build(g);
    build_present(g);
}

}