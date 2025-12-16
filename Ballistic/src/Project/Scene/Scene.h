#pragma once
#include "bepch.h"

namespace Ballistic {

    struct Parent { entt::entity entity = entt::null; };
    struct Children { std::vector<entt::entity> entities; };

    struct Tag { std::string name; };
    struct Sphere { float radius = 1.0f; };
    
    struct Transform {
        glm::vec3 position{0};
        glm::vec3 rotation{0};
        glm::vec3 scale{1};

        glm::mat4 TRS();
    };

    glm::mat4 ComputeWorldTransform(entt::registry& registry, entt::entity entity);

    class Scene {
    public:
        entt::entity create(const std::string& name, entt::entity parent = entt::null);
        void destroy(entt::entity entity);
        void reparent(entt::entity entity, entt::entity newParent = entt::null);
        void duplicate(entt::entity original);
        void duplicate(entt::entity original, entt::entity targetParent);

        void clear();

        entt::registry registry;
        entt::entity selected = entt::null;

    private:
        entt::entity duplicateEntity(entt::entity original, entt::entity targetParent);
    };

}