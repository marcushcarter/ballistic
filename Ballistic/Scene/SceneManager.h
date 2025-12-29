#pragma once
#include "bepch.h"

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

        std::shared_ptr<Scene> createScene(const std::string& name);
        void destroyScene(const std::string& name);

        Scene* GetActiveScene() { return m_activeScene; }
        bool HasActiveScene() const { return m_activeScene != nullptr; }

    private:
        std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
        Scene* m_activeScene = nullptr;
    };
    
} // namespace ballistic
