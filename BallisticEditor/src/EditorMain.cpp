#include <Ballistic.h>
#include <BallisticEntrypoint.h>
#include "EditorLayer.h"

namespace Ballistic
{

    class BallisticEditor : public Application {
    public:
        BallisticEditor() : Application() {

            m_LayerStack.pushLayer(std::make_shared<EditorLayer>(m_LayerStack, std::string("EditorLayer")));
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