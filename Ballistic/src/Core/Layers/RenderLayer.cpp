#include "RenderLayer.h"

namespace Ballistic {

    void RenderLayer::OnAttach() {
        m_Renderer.Init();
    }

    void RenderLayer::OnDetach() {
    }

    void RenderLayer::OnUpdate() {
        Image2D* image = m_Renderer.RenderComputeRtxStage();
        // call renderer->RenderScene();
    }

}