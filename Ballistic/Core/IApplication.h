#pragma once
#include "bepch.h"

namespace ballistic
{
    class Window;
    class LayerStack;

    struct LayerContext {
        LayerStack* layerStack = nullptr;
        Window* window = nullptr;
    };

    class IApplication
    {
    public:
        IApplication();        
        virtual ~IApplication() = default;

        bool Init();
        void Update(float deltaTime);
        void Shutdown();

        static void SetExeDirectory(const std::filesystem::path& path) { s_exeDir = path; }
        static const std::filesystem::path& GetExeDirectory() { return s_exeDir; }
        static std::filesystem::path GetResDirectory() { return s_exeDir / "Resources"; }
        
        std::shared_ptr<LayerStack> GetLayerStack() { return m_layerStack; }
    
    protected:
        virtual bool OnInit() = 0;
        virtual void OnUpdate(float deltaTime) = 0;
        virtual void OnShutdown() = 0;
        
        LayerContext CreateLayerContext() {
            LayerContext ctx;
            ctx.layerStack = m_layerStack.get();
            ctx.window = m_window.get();
            return ctx;
        }

        std::shared_ptr<LayerStack> m_layerStack;
        std::unique_ptr<Window> m_window;

    private:
        static inline std::filesystem::path s_exeDir;
    };
    
} // namespace ballistic

