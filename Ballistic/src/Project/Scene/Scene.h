#pragma once
#include "bepch.h"
#include "EntityHandle.h"
#include "Components.h"

namespace Ballistic {

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
        
        bool isDescendant(entt::entity ancestor, entt::entity potentialChild);

    private:
        entt::entity duplicateEntity(entt::entity original, entt::entity targetParent);
    };

}