#pragma once

#include "lrpch.h"
#include "Layer.h"
#include "../../Renderer/Renderer.h"

namespace Ballistic {

    class RenderLayer : public Layer {
    public:
        RenderLayer() = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate() override;

    private:
        Renderer m_Renderer;
    };

}