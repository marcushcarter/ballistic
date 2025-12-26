#include "Project/Scene/SceneManager.h"
#include "Project/Scene/Scene.h"

namespace Ballistic {

    std::shared_ptr<Scene> SceneManager::createScene(const std::string& name) {
        if (m_scenes.contains(name))
            throw std::runtime_error("Scene already exists: " + name);

        auto scene = std::make_shared<Scene>(name);
        m_scenes[name] = scene;

        if (!m_activeScene)
            m_activeScene = scene;

        return scene;
    }
    
    void SceneManager::destroyScene(const std::string& name) {
        auto it = m_scenes.find(name);
        if (it == m_scenes.end())
            return;

        if (m_activeScene == it->second)
            m_activeScene.reset();

        m_scenes.erase(it);

        if (!m_activeScene && !m_scenes.empty())
            m_activeScene = m_scenes.begin()->second;
    }
    
}