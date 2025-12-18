#pragma once
#include "bepch.h"
#include "EntityHandle.h"
#include "Components.h"

namespace Ballistic {

    class Scene {
    public:
        entt::entity create(const std::string& name, entt::entity parent = entt::null);
        void destroy(entt::entity entity);
        void reparent(entt::entity entity, entt::entity newParent = entt::null);
        void duplicate(entt::entity original);
        void duplicate(entt::entity original, entt::entity targetParent);
        
        bool isDescendant(entt::entity ancestor, entt::entity potentialChild);
        glm::mat4 ComputeWorldTransform(entt::entity entity);

        void clear();

        std::unordered_map<GUID, entt::entity> guidToEntity;
        entt::registry registry;
        entt::entity selected = entt::null;
        
        GUID GetGUID(entt::entity e) { return registry.get<GUID>(e); }
        entt::entity GetEntity(GUID id) {
            auto it = guidToEntity.find(id);
            return it != guidToEntity.end() ? it->second : entt::null;
        }

    private:
        entt::entity duplicateEntity(entt::entity original, entt::entity targetParent);
    };

}