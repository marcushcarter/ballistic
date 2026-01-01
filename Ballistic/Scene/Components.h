#pragma once
#include "bepch.h"
#include "Utility/GUID.h"

namespace ballistic
{
    struct Parent {
        GUID parent;
        Parent(GUID parent) : parent(parent) {}
    };

    struct Children {
        std::vector<GUID> children;
        Children(std::vector<GUID> children = {}) : children(children) {}
    };

    struct Tag {
        std::string name;
        Tag(std::string name = "New Node") : name(name) {}
    };

    struct SphereComponent {
        float radius = 1.0f;
        // SphereComponent(float radius = 1.0f) : radius(radius) {}
    };
    
    struct TransformComponent {
        glm::vec3 position{0};
        glm::vec3 rotation{0};
        glm::vec3 scale{1};

        // TransformComponent(glm::vec3 position = glm::vec3(0), glm::vec3 rotation = glm::vec3(0), glm::vec3 scale = glm::vec3(1))
        //     : position(position), rotation(rotation), scale(scale) {}

        glm::mat4 TRS() {
            glm::mat4 m(1.0f);
            m = glm::translate(m, position);
            glm::vec3 rot = glm::radians(rotation);
            m = glm::rotate(m, rot.z, glm::vec3(0, 0, 1));
            m = glm::rotate(m, rot.y, glm::vec3(0, 1, 0));
            m = glm::rotate(m, rot.x, glm::vec3(1, 0, 0));
            m = glm::scale(m, scale);
            return m;
        }
    };

    struct MeshComponent {
        GUID mesh;
        // MeshComponent(GUID meshID = GUID::Invalid) {
        //     guid.value = 0;
        // }
    };

    struct MaterialComponent {
        GUID material;
        // MaterialComponent(GUID materialID = GUID::Invalid) : guid.value(materialID) {}
    };

    struct CameraComponent {
        enum class Type { Perspective, Orthographic } type = Type::Perspective;

        float fov = 45.0f;
        float orthoSize = 10.0f;
        float aspectRatio = 16.0f / 9.0f;
        float nearPlane = 0.3f;
        float farPlane = 2000.0f;

        bool mainCamera = false;

        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        void OnUpdate(TransformComponent& transform) {
            glm::mat4 model = transform.TRS();
            view = glm::inverse(model);

            if (type == Type::Perspective) {
                projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
            } else {
                float half = orthoSize * 0.5f;
                projection = glm::ortho(-half * aspectRatio, half * aspectRatio, -half, half, nearPlane, farPlane);
            }
        }
    };

} // namespace ballistic
