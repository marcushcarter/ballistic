#include "Scene.h"

namespace Ballistic {

    glm::mat4 WorldTransform(entt::registry& registry, entt::entity entity) {
        glm::mat4 world(1.0f);
        entt::entity current = entity;

        while (current != entt::null) {
            if (registry.all_of<TransformComponent>(current)) {
                const TransformComponent& t = registry.get<TransformComponent>(current);
                world = t.GetMatrix() * world;
            }

            if (registry.all_of<Parent>(current)) {
                current = registry.get<Parent>(current).entity;
            } else {
                current = entt::null;
            }
        }

        return world;
    }

    entt::entity Scene::create(const std::string& tag, entt::entity parent) {
        auto entity = registry.create();
        registry.emplace<TagComponent>(entity, tag);

        if (parent != entt::null) {
            registry.emplace<Parent>(entity, parent);

            if (!registry.all_of<Children>(parent)) registry.emplace<Children>(parent);

            registry.get<Children>(parent).entities.push_back(entity);
        }

        return entity;
    }

    void Scene::destroy(entt::entity entity) {
        if (!registry.valid(entity)) return;

        if (registry.all_of<Children>(entity)) {
            auto childrenCopy = registry.get<Children>(entity).entities;
            for (auto child : childrenCopy) {
                destroy(child);
            }
            registry.remove<Children>(entity);
        }

        if (registry.all_of<Parent>(entity)) {
            entt::entity parent = registry.get<Parent>(entity).entity;
            if (registry.valid(parent) && registry.all_of<Children>(parent)) {
                auto& siblings = registry.get<Children>(parent).entities;
                siblings.erase(std::remove(siblings.begin(), siblings.end(), entity), siblings.end());
            }
            registry.remove<Parent>(entity);
        }

        registry.destroy(entity);
    }

    void Scene::reparent(entt::entity entity, entt::entity newParent) {
        if (!registry.valid(entity)) return;

        if (registry.all_of<Parent>(entity)) {
            auto oldParent = registry.get<Parent>(entity).entity;
            if (registry.valid(oldParent) && registry.all_of<Children>(oldParent)) {
                auto& siblings = registry.get<Children>(oldParent).entities;
                siblings.erase(std::remove(siblings.begin(), siblings.end(), entity), siblings.end());
            }
        }

        if (newParent == entt::null) {
            if (registry.all_of<Parent>(entity))
                registry.remove<Parent>(entity);
        } else {
            if (registry.all_of<Parent>(entity))
                registry.get<Parent>(entity).entity = newParent;
            else
                registry.emplace<Parent>(entity, newParent);

            if (!registry.all_of<Children>(newParent))
                registry.emplace<Children>(newParent);
            registry.get<Children>(newParent).entities.push_back(entity);
        }
    }

    void Scene::duplicate(entt::entity original) {
        if (!registry.valid(original)) return;

        std::function<entt::entity(entt::entity, entt::entity)> duplicateRec;
        duplicateRec = [&](entt::entity src, entt::entity parent) -> entt::entity {
            auto copy = registry.create();

            if (registry.all_of<TagComponent>(src)) {
                auto& tag = registry.get<TagComponent>(src);
                registry.emplace<TagComponent>(copy, tag.tag + " Copy");
            }

            if (parent != entt::null) {
                registry.emplace<Parent>(copy, parent);
                if (!registry.all_of<Children>(parent)) registry.emplace<Children>(parent);
                registry.get<Children>(parent).entities.push_back(copy);
            }

            if (registry.all_of<Children>(src)) {
                auto childrenCopy = registry.get<Children>(src).entities;
                for (auto child : childrenCopy) {
                    duplicateRec(child, copy);
                }
            }

            return copy;
        };

        entt::entity parent = registry.all_of<Parent>(original) ? registry.get<Parent>(original).entity : entt::null;
        duplicateRec(original, parent);
    }

    void Scene::duplicate(entt::entity original, entt::entity targetParent) {
        if (!registry.valid(original)) return;

        std::function<entt::entity(entt::entity, entt::entity)> duplicateRec;
        duplicateRec = [&](entt::entity src, entt::entity parent) -> entt::entity {
            auto copy = registry.create();

            if (registry.all_of<TagComponent>(src)) {
                auto& tag = registry.get<TagComponent>(src);
                registry.emplace<TagComponent>(copy, tag.tag + " Copy");
            }

            if (parent != entt::null) {
                registry.emplace<Parent>(copy, parent);
                if (!registry.all_of<Children>(parent))
                    registry.emplace<Children>(parent);
                registry.get<Children>(parent).entities.push_back(copy);
            }

            if (registry.all_of<Children>(src)) {
                auto childrenCopy = registry.get<Children>(src).entities;
                for (auto child : childrenCopy) {
                    duplicateRec(child, copy);
                }
            }

            return copy;
        };
    }

    void Scene::clear() {
        std::vector<entt::entity> roots;
        for (auto e : registry.view<TagComponent>()) {
            if (!registry.all_of<Parent>(e))
                roots.push_back(e);
        }

        for (auto root : roots)
            destroy(root);
    }

}
