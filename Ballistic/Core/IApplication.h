#pragma once
#include "bepch.h"
#include "Core/Layers/Layerstack.h"

namespace ballistic
{
    class Window;
    class Renderer;
    class WindowSettings;
    class SceneManager;

    struct LayerContext {
        LayerStack* layerStack = nullptr;
        Window* window = nullptr;
        Renderer* renderer = nullptr;
        SceneManager* sceneManager = nullptr;
    };

    class IApplication
    {
    public:
        IApplication() = default;
        virtual ~IApplication() = default;

        bool Init();
        void Update(float deltaTime);
        void Shutdown();

        static void SetExeDirectory(const std::filesystem::path& path) { s_exeDir = path; }
        static const std::filesystem::path& GetExeDirectory() { return s_exeDir; }
        static std::filesystem::path GetResDirectory() { return s_exeDir / "Resources"; }
    
    protected:
        virtual WindowSettings GetWindowSettings() const = 0;

        virtual bool OnInit() = 0;
        virtual void BeginFrame() = 0;
        virtual void OnUpdate(float deltaTime) = 0;
        virtual void EndFrame() = 0;
        virtual void OnShutdown() = 0;
        
        LayerContext CreateLayerContext() {
            return {
                .layerStack = &m_layerStack,
                .window = m_window.get(),
                .renderer = m_renderer.get(),
                .sceneManager = m_sceneManager.get()
            };
        }

        LayerStack m_layerStack;
        std::unique_ptr<SceneManager> m_sceneManager;

        std::unique_ptr<Window> m_window;
        std::unique_ptr<Renderer> m_renderer;
        
        LayerContext m_layerContext;

    private:
        static inline std::filesystem::path s_exeDir;
    };
    
} // namespace ballistic

