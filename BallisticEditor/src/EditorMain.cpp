#include <Ballistic.h>
#include <BallisticEntrypoint.h>
#include "EditorLayer.h"

namespace Ballistic {

    class BallisticEditor : public Application {
    public:
        BallisticEditor() : Application() {

            LayerContext context;
            context.layerStack = m_LayerStack;
            context.window = m_Window;
            context.renderer = m_OglRenderer;
            context.projectManager = m_ProjectManager;

            m_ImGuiContext = std::make_shared<ImGuiContext>(m_Window);
			m_ImGuiContext->Init();

            m_LayerStack->pushLayer(std::make_shared<EditorLayer>(context, m_ImGuiContext, std::string("EditorLayer")));
        }

        virtual void Shutdown() override {
            m_ImGuiContext->Shutdown();
            Application::Shutdown();
        }
        
        ~BallisticEditor() {
        }
    private:
        std::shared_ptr<ImGuiContext> m_ImGuiContext;
    };

    Application* CreateApplication(const std::filesystem::path& exeDir) {
        Config::Init(exeDir);
        return new BallisticEditor();
    }
}