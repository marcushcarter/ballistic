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
        m_activeScene = nullptr;
        m_scenes.clear();
    }

    std::shared_ptr<Scene> SceneManager::createScene(const std::string& name) {
        if (m_scenes.contains(name))
            throw std::runtime_error("Scene already exists: " + name);

        auto scene = std::make_shared<Scene>(name);
        m_scenes[name] = scene;

        if (!m_activeScene)
            m_activeScene = scene.get();

        return scene;
    }
    
    void SceneManager::destroyScene(const std::string& name) {
        auto it = m_scenes.find(name);
        if (it == m_scenes.end())
            return;

        Scene* scenePtr = it->second.get();

        if (m_activeScene == scenePtr)
            m_activeScene = nullptr;

        m_scenes.erase(it);

        // if (!m_activeScene && !m_scenes.empty())
        //     m_activeScene = m_scenes.begin()->second;
    }
    
} // namespace ballistic
