#include <core/rendering/render_path/render_path.h>
#include <core/log/log.h>

namespace ballistic {

Error RenderPath::create_resources()
{
    using enum Error;
    
    temp.create_resources();
    // post_process.create_resources();

    visibility.create_resources();
    geometry.create_resources();
    ao.create_resources();
    deferred_lighting.create_resources();
    subsurface_scattering.create_resources();

    return Ok;
}

void RenderPath::destroy_resources()
{
    temp.destroy_resources();
    // post_process.destroy_resources();

    visibility.destroy_resources();
    geometry.destroy_resources();
    ao.destroy_resources();
    deferred_lighting.destroy_resources();
    subsurface_scattering.destroy_resources();
}

void RenderPath::build(RenderGraph& g)
{
    // PASS CATEGORIES:
    // PostProcess.
    // Cull
    // DepthOnly (z pass).
    // Deferred.
    // Lighting.
    // Present. (anything that goes to the swapchain)

    temp.build(g);
    // post_process.build(g);

    visibility.build(g);
    geometry.build(g);
    ao.build(g);
    deferred_lighting.build(g);
    subsurface_scattering.build(g);

    build_present(g);
}

}