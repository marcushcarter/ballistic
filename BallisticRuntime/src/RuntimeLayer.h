#pragma once

#include "lrpch.h"
#include "Core/Layers/LayerStack.h"
#include "Core/Events/RenderEvents.h"

namespace Ballistic {

    class RuntimeLayer : public Layer {
    public:
        RuntimeLayer(std::shared_ptr<LayerStack> layerStack)
        : m_LayerStack(layerStack) {}

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate() override;
        void OnEvent(std::shared_ptr<IEvent> event) override;
        
    private:
        std::shared_ptr<LayerStack> m_LayerStack;
        
        // Image2D* m_CurrentFrame;
        // GLuint m_Framebuffer;
    };

}