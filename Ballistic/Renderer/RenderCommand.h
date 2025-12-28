#pragma once
#include "bepch.h"

namespace ballistic {

    struct RenderCommand {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
    };
}