#include <Ballistic.h>
#include <BallisticEntrypoint.h>
#include "EditorLayer.h"

namespace Ballistic
{

    class BallisticEditor : public Application {
    public:
        BallisticEditor() : Application() {

            LayerContext context;
            context.layerStack = m_LayerStack;
            context.window = m_Window;
            context.renderer = m_OglRenderer;
            context.projectManager = m_ProjectManager;

            m_LayerStack->pushLayer(std::make_shared<EditorLayer>(context, std::string("EditorLayer")));
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