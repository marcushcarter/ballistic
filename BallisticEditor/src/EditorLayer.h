#pragma once

#include "lrpch.h"
#include "Core/Layers/Layer.h"

namespace Ballistic {

    class EditorLayer : public Layer {
    public:
        EditorLayer() = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate() override;
    };

}