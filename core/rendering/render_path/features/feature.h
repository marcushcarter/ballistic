#pragma once
#include <core/rendering/render_graph.h>
#include <core/log/error.h>
#include <string>

namespace ballistic {

struct Feature
{
    std::string category = "?";
    bool enabled = true;

    virtual Error create_resources() { return Error::Ok; };
    virtual void destroy_resources() {}

    virtual void build(RenderGraph& g) = 0;

    virtual ~Feature() = default;
};

}