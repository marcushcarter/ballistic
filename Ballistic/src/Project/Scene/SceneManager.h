#pragma once
#include "bepch.h"

namespace Ballistic {

    class Scene;

    class SceneManager {
    public:
        std::shared_ptr<Scene> createScene(const std::string& name);
        void destroyScene(const std::string& name);

        Scene& GetActiveScene() { return *m_activeScene; }

    private:
        std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
        std::shared_ptr<Scene> m_activeScene = nullptr;
    };
    
}