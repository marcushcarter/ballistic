#include <Ballistic.h>
#include <BallisticEntrypoint.h>

namespace Ballistic
{

    class BallisticEditor : public Application {
    public:
        BallisticEditor()
            : Application() {
        }

        virtual void Shutdown() override {
            Application::Shutdown();
        }
        
        ~BallisticEditor() {
        }
    };

    Application* CreateApplication() {
        return new BallisticEditor();
    }
}