#include "Scene.h"

namespace Ballistic {

    entt::entity Scene::create(const std::string& name, entt::entity parent) {
        entt::entity eID = registry.create();
        EntityHandle e(eID, registry);

        GUID guid(GUID::Invalid);
        e.add<GUID>(guid);
        e.add<Tag>(name);
        
        guidToEntity[guid] = eID;
        
        if (parent != entt::null) {
            EntityHandle p(parent, registry);
            if (p.valid()) {
                e.add<Parent>(p.get<GUID>());

                if (!p.has<Children>())
                    p.add<Children>();

                p.get<Children>().children.push_back(guid);
            }
        }
        return eID;
    }

    void Scene::destroy(entt::entity id) {
        EntityHandle e(id, registry);
        if (!e.valid()) return;

        GUID guid = e.get<GUID>();

        if (e.has<Children>()) {
            auto children = e.get<Children>().children;
            for (GUID childGuid : children) {
                destroy(GetEntity(childGuid));
            }
            e.remove<Children>();
        }

        if (e.has<Parent>()) {
            GUID parentGuid = e.get<Parent>().parent;
            entt::entity parentEntity = GetEntity(parentGuid);
            if (parentEntity != entt::null) {
                EntityHandle p(parentEntity, registry);
                if (p.valid() && p.has<Children>()) {
                    auto& siblings = p.get<Children>().children;
                    siblings.erase(std::remove(siblings.begin(), siblings.end(), guid), siblings.end());
                }
            }
            e.remove<Parent>();
        }

        guidToEntity.erase(guid);
        registry.destroy(id);
    }

    bool Scene::isDescendant(entt::entity ancestor, entt::entity potentialChild) {
        EntityHandle a(ancestor, registry);
        if (!a.valid() || !a.has<Children>())
            return false;

        for (GUID child : a.get<Children>().children) {
            if (GetEntity(child) == potentialChild) return true;
            if (isDescendant(GetEntity(child), potentialChild)) return true;
        }

        return false;
    }

    void Scene::reparent(entt::entity entity, entt::entity newParent) {
        EntityHandle e(entity, registry);
        if (!e.valid() || entity == newParent) return;
        if (newParent != entt::null && isDescendant(entity, newParent))
            return;

        GUID guid = GetGUID(entity);

        if (e.has<Parent>()) {
            GUID oldParentGuid = e.get<Parent>().parent;
            entt::entity oldParentEntity = GetEntity(oldParentGuid);
            if (oldParentEntity != entt::null) {
                EntityHandle oldParent(oldParentEntity, registry);
                if (oldParent.valid() && oldParent.has<Children>()) {
                    auto& siblings = oldParent.get<Children>().children;
                    siblings.erase(std::remove(siblings.begin(), siblings.end(), guid), siblings.end());
                }
            }
            e.remove<Parent>();
        }

        if (newParent != entt::null) {
            EntityHandle np(newParent, registry);
            if (!np.valid()) return;

            GUID newParentGuid = GetGUID(newParent);
            e.add<Parent>(newParentGuid);

            if (!np.has<Children>())
                np.add<Children>();

            np.get<Children>().children.push_back(guid);
        }
    }

    entt::entity Scene::duplicateEntity(entt::entity original, entt::entity targetParent) {
        EntityHandle src(original, registry);
        if (!src.valid()) return entt::null;

        auto duplicateRec = [&](auto&& self, entt::entity srcID, entt::entity parentID) -> entt::entity {
            entt::entity copyID = registry.create();
            EntityHandle copy(copyID, registry);
            EntityHandle srcEnt(srcID, registry);

            GUID guid(GUID::Invalid);
            copy.add<GUID>(guid);
            guidToEntity[guid] = copyID;

            if (srcEnt.has<Tag>()) copy.add<Tag>(srcEnt.get<Tag>().name + " Copy");
            if (srcEnt.has<TransformComponent>()) copy.add<TransformComponent>(srcEnt.get<TransformComponent>());
            if (srcEnt.has<SphereComponent>()) copy.add<SphereComponent>(srcEnt.get<SphereComponent>());

            if (parentID != entt::null) {
                EntityHandle p(parentID, registry);
                GUID parentGuid = GetGUID(parentID);
                copy.add<Parent>(parentGuid);
                if (!p.has<Children>())
                    p.add<Children>();
                p.get<Children>().children.push_back(guid);
            }

            if (srcEnt.has<Children>()) {
                for (GUID childGuid : srcEnt.get<Children>().children) {
                    entt::entity childEnt = GetEntity(childGuid);
                    if (childEnt != entt::null)
                        self(self, childEnt, copyID);
                }
            }

            return copyID;
        };
        entt::entity parent = targetParent != entt::null ? targetParent : (src.has<Parent>() ? GetEntity(src.get<Parent>().parent) : entt::null);
        return duplicateRec(duplicateRec, original, parent);
    }

    void Scene::duplicate(entt::entity original) {
        EntityHandle e(original, registry);
        if (!e.valid()) return;
        entt::entity parent = e.has<Parent>() ? GetEntity(e.get<Parent>().parent) : entt::null;
        duplicateEntity(original, parent);
    }

    void Scene::duplicate(entt::entity original, entt::entity targetParent) {
        if (!registry.valid(original)) return;
        duplicateEntity(original, targetParent);
    }
    
    glm::mat4 Scene::ComputeWorldTransform(entt::entity entity) {
        glm::mat4 world(1.0f);
        entt::entity current = entity;
    
        while (current != entt::null && registry.valid(current)) {
            if (registry.all_of<TransformComponent>(current)) {
                TransformComponent& t = registry.get<TransformComponent>(current);
                world = t.TRS() * world;
            }
    
            if (registry.all_of<Parent>(current)) {
                GUID parentGUID = registry.get<Parent>(current).parent;
                entt::entity parent = GetEntity(parentGUID);
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

    void Scene::clear() {
        std::vector<entt::entity> roots;
        for (auto e : registry.view<Tag>()) {
            if (!registry.all_of<Parent>(e))
                roots.push_back(e);
        }

        for (auto root : roots)
            destroy(root);

        guidToEntity.clear();
    }

}