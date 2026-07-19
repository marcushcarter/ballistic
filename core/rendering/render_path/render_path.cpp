#include <core/rendering/render_path/render_path.h>
#include <core/log/log.h>

namespace ballistic {

Error RenderPath::create_resources()
{
    using enum Error;
    if (features.empty()) log_write("RenderPath: no features registered.");
    created_count = 0;
    for (size_t i = 0; i < features.size(); ++i) {
        features[i]->ctx = &ctx;
        if (Error e = features[i]->create_resources(); e != Ok) {
            destroy_resources();
            return e;
        }
        created_count = static_cast<uint32_t>(i + 1);
    }
    return Ok;
}

void RenderPath::destroy_resources()
{
    for (uint32_t i = created_count; i-- > 0; ) features[i]->destroy_resources();
    created_count = 0;
}

void RenderPath::build(RenderGraph& g)
{
    for (Feature* f : features) {
        if (f->enabled) f->build(g);
    }
}

}