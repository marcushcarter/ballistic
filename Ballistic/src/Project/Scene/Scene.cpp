#include "Scene.h"

namespace Ballistic {

    // glm::mat4 ComputeWorldTransform(entt::registry& registry, entt::entity entity) {
    //     glm::mat4 world(1.0f);
    //     entt::entity current = entity;
    //
    //     while (current != entt::null && registry.valid(current)) {
    //         if (registry.all_of<TransformComponent>(current)) {
    //             TransformComponent& t = registry.get<TransformComponent>(current);
    //             world = t.TRS() * world;
    //         }
    //
    //         if (registry.all_of<Parent>(current)) {
    //             entt::entity parent = registry.get<Parent>(current).entity;
    //             if (registry.valid(parent))
    //                 current = parent;
    //             else
    //                 break;
    //         } else {
    //             break;
    //         }
    //     }
    //
    //     return world;
    // }

    entt::entity Scene::create(const std::string& name, entt::entity parent) {
        entt::entity eID = registry.create();
        EntityHandle e(eID, registry);

        e.add<Tag>(name);
        e.add<GUID>(GUID::Invalid);
        
        if (parent != entt::null) {
            EntityHandle p(parent, registry);
            if (p.valid()) {
                e.add<Parent>(parent);

                if (!p.has<Children>())
                    p.add<Children>();

                p.get<Children>().entities.push_back(eID);
            }
        }
        return eID;
    }

    void Scene::destroy(entt::entity id) {
        EntityHandle e(id, registry);
        if (!e.valid()) return;

        if (e.has<Children>()) {
            auto children = e.get<Children>().entities;
            for (entt::entity child : children)
                destroy(child);
            e.remove<Children>();
        }

        if (e.has<Parent>()) {
            EntityHandle p(e.get<Parent>().entity, registry);
            if (p.valid() && p.has<Children>()) {
                auto& siblings = p.get<Children>().entities;
                siblings.erase(std::remove(siblings.begin(), siblings.end(), id), siblings.end());
            }
            e.remove<Parent>();
        }

        registry.destroy(id);
    }

    bool Scene::isDescendant(entt::entity ancestor, entt::entity potentialChild) {
        EntityHandle a(ancestor, registry);
        if (!a.valid() || !a.has<Children>())
            return false;

        for (entt::entity child : a.get<Children>().entities) {
            if (child == potentialChild)
                return true;
            if (isDescendant(child, potentialChild))
                return true;
        }

        return false;
    }

    void Scene::reparent(entt::entity entity, entt::entity newParent) {
        EntityHandle e(entity, registry);
        if (!e.valid() || entity == newParent) return;
        if (newParent != entt::null && isDescendant(entity, newParent))
            return;

        if (e.has<Parent>()) {
            EntityHandle oldParent(e.get<Parent>().entity, registry);
            if (oldParent.valid() && oldParent.has<Children>()) {
                auto& siblings = oldParent.get<Children>().entities;
                siblings.erase(std::remove(siblings.begin(), siblings.end(), entity), siblings.end());
            }
            e.remove<Parent>();
        }

        if (newParent != entt::null) {
            EntityHandle np(newParent, registry);
            if (!np.valid()) return;
            e.add<Parent>(newParent);
            if (!np.has<Children>())
                np.add<Children>();
            np.get<Children>().entities.push_back(entity);
        }
    }

    entt::entity Scene::duplicateEntity(entt::entity original, entt::entity targetParent) {
        EntityHandle src(original, registry);
        if (!src.valid()) return entt::null;

        auto duplicateRec = [&](auto&& self, entt::entity srcID, entt::entity parentID) -> entt::entity {
            entt::entity copyID = registry.create();
            EntityHandle copy(copyID, registry);
            EntityHandle srcEnt(srcID, registry);

            if (srcEnt.has<Tag>()) copy.add<Tag>(srcEnt.get<Tag>().name + " Copy");
            if (srcEnt.has<GUID>()) copy.add<GUID>(srcEnt.get<GUID>().value);
            if (srcEnt.has<TransformComponent>()) copy.add<TransformComponent>(srcEnt.get<TransformComponent>());
            if (srcEnt.has<SphereComponent>()) copy.add<SphereComponent>(srcEnt.get<SphereComponent>());

            if (parentID != entt::null) {
                EntityHandle p(parentID, registry);
                copy.add<Parent>(parentID);
                if (!p.has<Children>())
                    p.add<Children>();
                p.get<Children>().entities.push_back(copyID);
            }

            if (srcEnt.has<Children>()) {
                for (entt::entity child : srcEnt.get<Children>().entities) self(self, child, copyID);
            }
            return copyID;
        };
        entt::entity parent = targetParent != entt::null ? targetParent : (src.has<Parent>() ? src.get<Parent>().entity : entt::null);
        return duplicateRec(duplicateRec, original, parent);
    }

    void Scene::duplicate(entt::entity original) {
        EntityHandle e(original, registry);
        if (!e.valid()) return;
        entt::entity parent = e.has<Parent>() ? e.get<Parent>().entity : entt::null;
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