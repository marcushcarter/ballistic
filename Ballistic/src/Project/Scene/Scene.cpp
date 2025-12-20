#include "Scene.h"
#include "EntityHandle.h"
#include "Components.h"
#include "Core/GUID.h"

namespace Ballistic {

    entt::entity Scene::Create(const std::string& name, entt::entity parent) {
        entt::entity eID = m_registry.create();
        EntityHandle e(eID, m_registry);

        GUID guid(GUID::Invalid);
        e.add<GUID>(guid);
        e.add<Tag>(name);
        
        guidToEntity[guid] = eID;
        
        if (parent != entt::null) {
            EntityHandle p(parent, m_registry);
            if (p.valid()) {
                e.add<Parent>(p.get<GUID>());

                if (!p.has<Children>())
                    p.add<Children>();

                p.get<Children>().children.push_back(guid);
            }
        }
        return eID;
    }

    void Scene::Destroy(entt::entity id) {
        EntityHandle e(id, m_registry);
        if (!e.valid()) return;

        GUID guid = e.get<GUID>();

        if (e.has<Children>()) {
            auto children = e.get<Children>().children;
            for (GUID childGuid : children) {
                Destroy(GetEntity(childGuid));
            }
            e.remove<Children>();
        }

        if (e.has<Parent>()) {
            GUID parentGuid = e.get<Parent>().parent;
            entt::entity parentEntity = GetEntity(parentGuid);
            if (parentEntity != entt::null) {
                EntityHandle p(parentEntity, m_registry);
                if (p.valid() && p.has<Children>()) {
                    auto& siblings = p.get<Children>().children;
                    siblings.erase(std::remove(siblings.begin(), siblings.end(), guid), siblings.end());
                }
            }
            e.remove<Parent>();
        }

        guidToEntity.erase(guid);
        m_registry.destroy(id);
    }

    void Scene::Clear() {
        std::vector<entt::entity> roots;
        for (auto e : m_registry.view<Tag>()) {
            if (!m_registry.all_of<Parent>(e))
                roots.push_back(e);
        }

        for (auto root : roots)
            Destroy(root);

        guidToEntity.clear();
    }

    void Scene::Reparent(entt::entity entity, entt::entity newParent) {
        EntityHandle e(entity, m_registry);
        if (!e.valid() || entity == newParent) return;
        if (newParent != entt::null && IsDescendant(entity, newParent))
            return;

        GUID guid = GetGUID(entity);

        if (e.has<Parent>()) {
            GUID oldParentGuid = e.get<Parent>().parent;
            entt::entity oldParentEntity = GetEntity(oldParentGuid);
            if (oldParentEntity != entt::null) {
                EntityHandle oldParent(oldParentEntity, m_registry);
                if (oldParent.valid() && oldParent.has<Children>()) {
                    auto& siblings = oldParent.get<Children>().children;
                    siblings.erase(std::remove(siblings.begin(), siblings.end(), guid), siblings.end());
                }
            }
            e.remove<Parent>();
        }

        if (newParent != entt::null) {
            EntityHandle np(newParent, m_registry);
            if (!np.valid()) return;

            GUID newParentGuid = GetGUID(newParent);
            e.add<Parent>(newParentGuid);

            if (!np.has<Children>())
                np.add<Children>();

            np.get<Children>().children.push_back(guid);
        }
    }

    bool Scene::IsDescendant(entt::entity ancestor, entt::entity potentialChild) {
        EntityHandle a(ancestor, m_registry);
        if (!a.valid() || !a.has<Children>())
            return false;

        for (GUID child : a.get<Children>().children) {
            if (GetEntity(child) == potentialChild) return true;
            if (IsDescendant(GetEntity(child), potentialChild)) return true;
        }

        return false;
    }

    entt::entity Scene::duplicateEntity(entt::entity original, entt::entity targetParent) {
        EntityHandle src(original, m_registry);
        if (!src.valid()) return entt::null;

        auto duplicateRec = [&](auto&& self, entt::entity srcID, entt::entity parentID) -> entt::entity {
            entt::entity copyID = m_registry.create();
            EntityHandle copy(copyID, m_registry);
            EntityHandle srcEnt(srcID, m_registry);

            GUID guid(GUID::Invalid);
            copy.add<GUID>(guid);
            guidToEntity[guid] = copyID;

            if (srcEnt.has<Tag>()) copy.add<Tag>(srcEnt.get<Tag>().name + " Copy");
            if (srcEnt.has<TransformComponent>()) copy.add<TransformComponent>(srcEnt.get<TransformComponent>());
            if (srcEnt.has<SphereComponent>()) copy.add<SphereComponent>(srcEnt.get<SphereComponent>());
            if (srcEnt.has<MeshComponent>()) copy.add<MeshComponent>(srcEnt.get<MeshComponent>());
            if (srcEnt.has<MaterialComponent>()) copy.add<MaterialComponent>(srcEnt.get<MaterialComponent>());
            if (srcEnt.has<CameraComponent>()) copy.add<CameraComponent>(srcEnt.get<CameraComponent>());

            if (parentID != entt::null) {
                EntityHandle p(parentID, m_registry);
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

    void Scene::Duplicate(entt::entity original) {
        EntityHandle e(original, m_registry);
        if (!e.valid()) return;
        entt::entity parent = e.has<Parent>() ? GetEntity(e.get<Parent>().parent) : entt::null;
        duplicateEntity(original, parent);
    }

    void Scene::Duplicate(entt::entity original, entt::entity targetParent) {
        if (!m_registry.valid(original)) return;
        duplicateEntity(original, targetParent);
    }
    
    glm::mat4 Scene::ComputeWorldTransform(entt::entity entity) {
        glm::mat4 world(1.0f);
        entt::entity current = entity;
    
        while (current != entt::null && m_registry.valid(current)) {
            if (m_registry.all_of<TransformComponent>(current)) {
                TransformComponent& t = m_registry.get<TransformComponent>(current);
                world = t.TRS() * world;
            }
    
            if (m_registry.all_of<Parent>(current)) {
                GUID parentGUID = m_registry.get<Parent>(current).parent;
                entt::entity parent = GetEntity(parentGUID);
                if (m_registry.valid(parent))
                    current = parent;
                else
                    break;
            } else {
                break;
            }
        }
    
        return world;
    }

}