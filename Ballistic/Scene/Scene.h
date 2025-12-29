#pragma once
#include "bepch.h"
#include "Utility/GUID.h"

namespace ballistic
{
    struct GUID;
    struct CameraComponent;

    class Scene
    {
    public:
        Scene(const std::string& sceneName = "New Scene")
            : m_guid(GUID::Create()), m_name(sceneName) {}

        entt::entity Create(const std::string& name, entt::entity parent = entt::null);
        void Destroy(entt::entity entity);
        void Clear();

        void Reparent(entt::entity entity, entt::entity newParent = entt::null);
        bool IsDescendant(entt::entity ancestor, entt::entity potentialChild);

        void Duplicate(entt::entity original);
        void Duplicate(entt::entity original, entt::entity targetParent);

        std::vector<entt::entity> GetAllEntitiesFlattened();
        
        glm::mat4 ComputeWorldTransform(entt::entity entity);
        void SetMainCamera(entt::entity entity);
        CameraComponent* GetMainCamera();
        
        GUID ConvertGUID(entt::entity e) { return m_registry.get<GUID>(e); }
        entt::entity ConvertEntity(GUID id) { auto it = guidToEntity.find(id); return it != guidToEntity.end() ? it->second : entt::null; }

        void SetSelected(entt::entity entity) { m_selected = entity; }
        entt::entity GetSelected() const { return m_selected; }
        entt::entity& GetSelected() { return m_selected; }

        entt::registry& GetRegistry() { return m_registry; }

        const GUID& GetGUID() const { return m_guid; }
        void SetName(const std::string& name) { m_name = name; }
        const std::string& GetName() const { return m_name; }

    private:
        GUID m_guid;
        std::string m_name;

        std::unordered_map<GUID, entt::entity> guidToEntity;
        entt::registry m_registry;
        entt::entity m_selected = entt::null;
        
        entt::entity duplicateEntity(entt::entity original, entt::entity targetParent);
    };

} // namespace ballistic
