#pragma once
#include <Ballistic.h>

namespace ballistic
{

    class EditorCamera
    {
    public:
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec3 position;

        glm::vec3 target = glm::vec3(0.0f);
        float distance = 20.0f;
        float pitch = 0.0f;
        float yaw = 0.0f;
        float fov = 60.0f;
        glm::vec2 viewportSize;

        void SetViewportSize(const glm::vec2& size);

        void UpdateView();
        void UpdateFromMouse(const glm::vec2& mouseDelta, int mode);
    };

} // namespace ballistic
