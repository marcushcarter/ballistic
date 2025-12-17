#pragma once
#include "bepch.h"
#include "Core/GUID.h"

namespace Ballistic {

    struct Parent {
        entt::entity entity = entt::null;
        Parent(entt::entity entity = entt::null) : entity(entity) {}
    };

    struct Children {
        std::vector<entt::entity> entities;
        Children(std::vector<entt::entity> entities = {}) : entities(entities) {}
    };

    struct Tag {
        std::string name;
        Tag(std::string name = "null") : name(name) {}
    };

    struct SphereComponent {
        float radius = 1.0f;
        SphereComponent(float radius = 1.0f) : radius(radius) {}
    };
    
    struct TransformComponent {
        glm::vec3 position{0};
        glm::vec3 rotation{0};
        glm::vec3 scale{1};

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

}