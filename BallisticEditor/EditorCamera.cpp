#include "EditorCamera.h"

namespace ballistic
{
    void EditorCamera::SetViewportSize(const glm::vec2& size) {
        viewportSize = size;
        float aspect = size.x / size.y;
        projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 1000.0f);
        UpdateView();
    }

    void EditorCamera::UpdateView() {
        position.x = target.x + distance * cosf(pitch) * sinf(yaw);
        position.y = target.y + distance * sinf(pitch);
        position.z = target.z + distance * cosf(pitch) * cosf(yaw);
        view = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void EditorCamera::UpdateFromMouse(const glm::vec2& mouseDelta, int mode) {
        const float rotateSpeed = 0.005f;
        const float panSpeed = 0.002f * distance;
        const float zoomSpeed = 0.1f;

        if (mode == 0) {
            yaw   -= mouseDelta.x * rotateSpeed;
            pitch -= mouseDelta.y * rotateSpeed;
            pitch = glm::clamp(pitch, -1.5f, 1.5f);
        } 
        else if (mode == 1) {
            glm::vec3 forward = glm::normalize(target - position);
            glm::vec3 right   = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 up      = glm::normalize(glm::cross(right, forward));
            target += -right * mouseDelta.x * panSpeed + up * mouseDelta.y * panSpeed;
        } 
        else if (mode == 2) {
            distance *= 1.0f - mouseDelta.y * zoomSpeed;
            distance = std::max(distance, 0.1f);
        }

        UpdateView();
    }
    
} // namespace ballistic
