#pragma once
#include "bepch.h"
#include "Utility/GUID.h"

namespace ballistic
{
    class Scene;

    class SceneManager
    {
    public:
        SceneManager() = default;
        ~SceneManager() { Shutdown(); }

        bool Init();
        void Shutdown();

        std::shared_ptr<Scene> Create(const std::string& name);
        void Destroy(GUID guid);

        Scene* ConvertScene(GUID guid);
        GUID ConvertGUID(Scene* scene) const;

        void SetActiveScene(GUID guid);
        Scene* GetActiveScene();
        bool HasActiveScene() const { return m_activeScene != GUID::Invalid; }

        std::vector<std::shared_ptr<Scene>> GetAllScenes() const;

    private:
        std::unordered_map<GUID, std::shared_ptr<Scene>> m_guidToScene;
        GUID m_activeScene = GUID::Invalid;
    };
}
