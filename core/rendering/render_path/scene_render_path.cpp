#include <core/rendering/render_path/scene_render_path.h>
#include <core/log/log.h>

namespace ballistic {

Error SceneRenderPath::create_resources()
{
    using enum Error;

    Feature* features[] = { &temp, &visibility, &geometry, &ao, &deferred_lighting, &subsurface_scattering };
    for (Feature* f : features) {
        f->dd = dd;
        f->graph = graph;
        if (Error e = f->create_resources(); e != Ok) return e;
    }

    return Ok;
}

void SceneRenderPath::destroy_resources()
{
    temp.destroy_resources();

    visibility.destroy_resources();
    geometry.destroy_resources();
    ao.destroy_resources();
    deferred_lighting.destroy_resources();
    subsurface_scattering.destroy_resources();
}

void SceneRenderPath::build(RenderGraph& g)
{
    temp.build(g);

    visibility.build(g);
    geometry.build(g);
    ao.build(g);
    deferred_lighting.build(g);
    subsurface_scattering.build(g);
    build_present(g);
}

}