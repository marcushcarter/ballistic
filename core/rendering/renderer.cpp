#include <core/rendering/renderer.h>
#include <core/error/error_macros.h>

namespace ballistic {

Error Renderer::create(uint32_t p_frame_count)
{
    (void)p_frame_count;
    using enum Error;

    return Ok;
}

void Renderer::destroy()
{

}

Error Renderer::begin_frame()
{
    using enum Error;

    return Ok;
}

Error Renderer::end_frame()
{
    using enum Error;

    return Ok;
}

}