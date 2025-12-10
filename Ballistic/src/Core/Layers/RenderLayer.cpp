#include "RenderLayer.h"

#include "Core/Events/RenderEvents.h"

namespace Ballistic {

    void RenderLayer::OnAttach() {
        m_Renderer.Init();
    }

    void RenderLayer::OnDetach() {
    }

    void RenderLayer::OnUpdate() {
        auto image = m_Renderer.RenderComputeRtxStage();
        std::shared_ptr<IEvent> event = std::make_shared<FrameRenderedEvent>(image);

        if (m_LayerStack) {
            m_LayerStack->DispatchEvent(event);
        }
    }

}