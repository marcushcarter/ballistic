#pragma once
#include "bepch.h"

namespace ballistic
{
    class LayerStack;

    struct LayerContext {
        std::shared_ptr<LayerStack> layerStack;
    };

    class IApplication
    {
    public:
        IApplication();        
        virtual ~IApplication() = default;

        virtual bool Init() = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void Shutdown() = 0;

        void SetExeDirectory(const std::filesystem::path& path) { m_exeDir = path; }
        const std::filesystem::path& GetExeDirectory() const { return m_exeDir; }
        
        std::shared_ptr<LayerStack> GetLayerStack() { return m_layerStack; }

    private:
        std::filesystem::path m_exeDir;

        std::shared_ptr<LayerStack> m_layerStack;
    };
    
} // namespace ballistic

