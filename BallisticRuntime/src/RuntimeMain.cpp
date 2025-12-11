#include <Ballistic.h>
#include <BallisticEntrypoint.h>

namespace Ballistic
{

    class BallisticRuntime : public Application {
    public:
        BallisticRuntime() : Application() {
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