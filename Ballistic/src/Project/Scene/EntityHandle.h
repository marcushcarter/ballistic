#pragma once
#include "bepch.h"

namespace Ballistic {

    class EntityHandle {
    public:
        EntityHandle(entt::entity handle, entt::registry& registry)
            : entity(handle), registry(registry) {}

        template<typename T, typename... Args>
        T& add(Args&&... args) { return registry.emplace<T>(entity, std::forward<Args>(args)...); }

        template<typename T>
        T& get() { return registry.get<T>(entity); }

        template<typename T>
        bool has() const { return registry.all_of<T>(entity); }

        template<typename T>
        void remove() { registry.remove<T>(entity); }

        entt::entity handle() const { return entity; }
        entt::entity& handle() { return entity; }
        bool valid() const { return entity != entt::null && registry.valid(entity); }

    private:
        entt::entity entity = entt::null;
        entt::registry& registry;
    };
}