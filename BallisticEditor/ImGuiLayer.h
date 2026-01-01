#pragma once
#include <Ballistic.h>

struct ImGuiContext;

namespace ballistic
{
    class ImGuiLayer
    {
    public:
        ImGuiLayer(LayerContext& context);
        ~ImGuiLayer() { Shutdown(); }

        void Init();
        void Shutdown();
        void BeginFrame();
        void EndFrame();

    private:
        LayerContext& m_context;
    };
    
} // namespace ballistic
