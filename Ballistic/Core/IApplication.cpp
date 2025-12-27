#include "Core/IApplication.h"
#include "Core/Layers/LayerStack.h"

namespace ballistic
{
    IApplication::IApplication() {
        m_layerStack = std::make_shared<LayerStack>();
    }
} // namespace ballistic
