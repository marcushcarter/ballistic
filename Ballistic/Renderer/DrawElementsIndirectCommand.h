#pragma once
#include "bepch.h"

namespace ballistic
{
    struct DrawElementsIndirectCommand {
        uint32_t count;
        uint32_t instanceCount;
        uint32_t firstIndex;
        int32_t  baseVertex;
        uint32_t baseInstance;
    };

} // namespace ballistic
