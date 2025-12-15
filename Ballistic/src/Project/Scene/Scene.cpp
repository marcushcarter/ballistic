#include "Scene.h"

namespace Ballisic {

    glm::mat4 Transform::TRS() {
        glm::mat4 m(1.0f);

        m = glm::translate(m, position);

        glm::vec3 rot = glm::radians(rotation);

        m = glm::rotate(m, rot.z, glm::vec3(0, 0, 1));
        m = glm::rotate(m, rot.y, glm::vec3(0, 1, 0));
        m = glm::rotate(m, rot.x, glm::vec3(1, 0, 0));

        m = glm::scale(m, scale);

        return m;
    }

    glm::mat4 ComputeWorldTransform(entt::registry& registry, entt::entity entity) {
        glm::mat4 world(1.0f);
        entt::entity current = entity;

        while (current != entt::null && registry.valid(current)) {
            if (registry.all_of<Transform>(current)) {
                Transform& t = registry.get<Transform>(current);
                world = t.TRS() * world;
            }

            if (registry.all_of<Parent>(current)) {
                entt::entity parent = registry.get<Parent>(current).entity;
                if (registry.valid(parent))
                    current = parent;
                else
                    break;
            } else {
                break;
            }
        }

        return world;
    }

    entt::entity Scene::create(const std::string& name, entt::entity parent) {
        auto entity = registry.create();
        registry.emplace<Tag>(entity, name);

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

    entt::entity Scene::duplicateEntity(entt::entity original, entt::entity targetParent) {
        if (!registry.valid(original)) return entt::null;

        std::function<entt::entity(entt::entity, entt::entity)> duplicateRec;
        duplicateRec = [&](entt::entity src, entt::entity parent) -> entt::entity {
            auto copy = registry.create();

            if (registry.all_of<Tag>(src))
                registry.emplace<Tag>(copy, registry.get<Tag>(src).name + " Copy");

            if (registry.all_of<Transform>(src))
                registry.emplace<Transform>(copy, registry.get<Transform>(src));

            if (registry.all_of<Sphere>(src))
                registry.emplace<Sphere>(copy, registry.get<Sphere>(src));

            if (parent != entt::null) {
                registry.emplace<Parent>(copy, parent);
                if (!registry.all_of<Children>(parent))
                    registry.emplace<Children>(parent);
                registry.get<Children>(parent).entities.push_back(copy);
            }

            if (registry.all_of<Children>(src)) {
                for (auto child : registry.get<Children>(src).entities) {
                    duplicateRec(child, copy);
                }
            }

            return copy;
        };

        entt::entity parent = targetParent != entt::null ? targetParent : (registry.all_of<Parent>(original) ? registry.get<Parent>(original).entity : entt::null);
        return duplicateRec(original, parent);
    }

    void Scene::duplicate(entt::entity original) {
        if (!registry.valid(original)) return;
        entt::entity parent = registry.all_of<Parent>(original) ? registry.get<Parent>(original).entity : entt::null;
        duplicateEntity(original, parent);
    }

    void Scene::duplicate(entt::entity original, entt::entity targetParent) {
        if (!registry.valid(original)) return;
        duplicateEntity(original, targetParent);
    }

    void Scene::clear() {
        std::vector<entt::entity> roots;
        for (auto e : registry.view<Tag>()) {
            if (!registry.all_of<Parent>(e))
                roots.push_back(e);
        }

        for (auto root : roots)
            destroy(root);
    }

}