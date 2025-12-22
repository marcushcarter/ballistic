#include <Ballistic.h>
#include <Entry/BallisticEntrypoint.h>
#include "EditorLayer.h"

namespace Ballistic {

    class BallisticEditor : public Application {
    public:
        BallisticEditor() 
            : Application(WindowPresets::Editor()) {

            m_layerStack->PushLayer(std::make_shared<EditorLayer>(GetAppContext(), std::string("EditorLayer")));
        }

        virtual void Shutdown() override {
            Application::Shutdown();
        }
        
        ~BallisticEditor() {
        }
    };

    Application* CreateApplication(const std::filesystem::path& exeDir) {
        Config::Init(exeDir);
        return new BallisticEditor();
    }
}