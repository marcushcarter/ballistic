#pragma once
#include "bepch.h"

namespace ballistic
{
    class Window;
    class LayerStack;
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
        
        std::shared_ptr<LayerStack> GetLayerStack() { return m_layerStack; }
    
    protected:
        virtual WindowSettings GetWindowSettings() const = 0;
        virtual bool OnInit() = 0;
        virtual void OnUpdate(float deltaTime) = 0;
        virtual void OnShutdown() = 0;
        
        LayerContext CreateLayerContext() {
            LayerContext ctx;
            ctx.layerStack = m_layerStack.get();
            ctx.window = m_window.get();
            ctx.renderer = m_renderer.get();
            ctx.sceneManager = m_sceneManager.get();
            return ctx;
        }

        std::shared_ptr<LayerStack> m_layerStack;
        std::unique_ptr<SceneManager> m_sceneManager;

        std::unique_ptr<Window> m_window;
        std::unique_ptr<Renderer> m_renderer;
        
        LayerContext m_layerContext;

    private:
        static inline std::filesystem::path s_exeDir;
    };
    
} // namespace ballistic

