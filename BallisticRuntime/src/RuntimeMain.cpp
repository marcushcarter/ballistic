#include <Ballistic.h>
#include <BallisticEntrypoint.h>
#include "RuntimeLayer.h"

namespace Ballistic
{

    class BallisticRuntime : public Application {
    public:
        BallisticRuntime() : Application() {

            m_LayerStack.pushLayer(std::make_shared<RuntimeLayer>("RuntimeLayer"));
        }

        virtual void Shutdown() override {
            Application::Shutdown();
        }
        
        ~BallisticRuntime() {
        }
    };

    Application* CreateApplication() {
        return new BallisticRuntime();
    }
}