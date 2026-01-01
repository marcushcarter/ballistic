#pragma once
#include "bepch.h"
#include "Utility/GUID.h"

namespace ballistic
{
    struct MeshMetadata {
        GUID guid;
        std::string name;
        size_t vertexOffset;
        size_t vertexCount;
        size_t indexOffset;
        size_t indexCount;
    };

} // namespace ballistic
