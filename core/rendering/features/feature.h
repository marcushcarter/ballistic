#pragma once
#include <core/rendering/render_context.h>
#include <core/rendering/render_graph.h>
#include <core/log/error.h>
#include <string>

namespace ballistic {

struct Feature
{
    const RenderContext* ctx = nullptr;
    
    std::string category = "?";
    bool enabled = true;

    virtual Error create_resources() { return Error::Ok; };
    virtual Error create_pipelines() { return Error::Ok; }
    virtual void destroy_resources() {}
    virtual void build(RenderGraph& g) = 0;
    
    virtual ~Feature() = default;
};

}