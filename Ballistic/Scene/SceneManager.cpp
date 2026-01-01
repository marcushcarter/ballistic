#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Root/LogManager/Log.h"

namespace ballistic
{
    bool SceneManager::Init() {
        LogDebug("Scene manager initialized");
        return true;
    }

    void SceneManager::Shutdown() {
        m_activeScene = GUID::Invalid;
        m_guidToScene.clear();
    }

    std::shared_ptr<Scene> SceneManager::Create(const std::string& name) {
        auto scene = std::make_shared<Scene>(name);
        GUID guid = scene->GetGUID();

        if (m_guidToScene.contains(guid))
            throw std::runtime_error("Scene GUID already exists");

        m_guidToScene[guid] = scene;

        if (m_activeScene == GUID::Invalid)
            m_activeScene = guid;

        return scene;
    }

    void SceneManager::Destroy(GUID guid) {
        auto it = m_guidToScene.find(guid);
        if (it == m_guidToScene.end())
            return;

        if (m_activeScene == guid)
            m_activeScene = GUID::Invalid;

        m_guidToScene.erase(it);
    }

    Scene* SceneManager::ConvertScene(GUID guid) {
        auto it = m_guidToScene.find(guid);
        return it != m_guidToScene.end() ? it->second.get() : nullptr;
    }

    GUID SceneManager::ConvertGUID(Scene* scene) const {
        if (!scene) return GUID::Invalid;
        return scene->GetGUID();
    }

    void SceneManager::SetActiveScene(GUID guid) {
        if (m_guidToScene.contains(guid))
            m_activeScene = guid;
    }

    Scene* SceneManager::GetActiveScene() {
        auto it = m_guidToScene.find(m_activeScene);
        return it != m_guidToScene.end() ? it->second.get() : nullptr;
    }

    std::vector<std::shared_ptr<Scene>> SceneManager::GetAllScenes() const {
        std::vector<std::shared_ptr<Scene>> scenes;
        for (const auto& [guid, scene] : m_guidToScene)
            scenes.push_back(scene);
        return scenes;
    }
}
