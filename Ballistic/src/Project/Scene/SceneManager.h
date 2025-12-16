#pragma once
#include "bepch.h"
#include "Scene.h"

namespace Ballistic {

    class SceneManager {
    public:
        SceneManager() = default;

        std::shared_ptr<Scene> createScene(const std::string& name) {
            if (m_scenes.contains(name))
                throw std::runtime_error("Scene already exists: " + name);

            auto scene = std::make_shared<Scene>();
            m_scenes[name] = scene;

            // if (!m_activeScene)
                m_activeScene = scene;

            return scene;
        }
        
        void destroyScene(const std::string& name) {
            auto it = m_scenes.find(name);
            if (it == m_scenes.end())
                return;

            if (m_activeScene == it->second)
                m_activeScene.reset();

            m_scenes.erase(it);

            if (!m_activeScene && !m_scenes.empty())
                m_activeScene = m_scenes.begin()->second;
        }

        void setActiveScene(const std::string& name);
        std::shared_ptr<Scene> getActiveScene() const;
        
        std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
        std::shared_ptr<Scene> m_activeScene = nullptr;

    private:
    };
    
}