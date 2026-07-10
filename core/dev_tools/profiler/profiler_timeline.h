#pragma once
#include <core/dev_tools/dev_context.h>

namespace ballistic {

struct ProfilerSelection
{
    const char* name = nullptr;
    const char* category = nullptr;
    float ms = 0.0f;

    bool valid() const { return name != nullptr; }
};

struct ProfilerTimeline
{
    void draw(DevContext& ctx);
};

}