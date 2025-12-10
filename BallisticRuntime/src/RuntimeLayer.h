#pragma once

#include "lrpch.h"
#include "Core/Layers/Layer.h"

namespace Ballistic {

    class RuntimeLayer : public Layer {
    public:
        RuntimeLayer() = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate() override;
    };

}