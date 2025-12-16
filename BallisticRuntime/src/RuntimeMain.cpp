#include <Ballistic.h>
#include <BallisticEntrypoint.h>
#include "RuntimeLayer.h"

namespace Ballistic {

    class BallisticRuntime : public Application {
    public:
        BallisticRuntime() : Application() {

            LayerContext context;
            context.layerStack = m_LayerStack;
            context.window = m_Window;
            context.renderer = m_OglRenderer;
            context.projectManager = m_ProjectManager;

            m_LayerStack->pushLayer(std::make_shared<RuntimeLayer>(context, std::string("RuntimeLayer")));
        }

        virtual void Shutdown() override {
            Application::Shutdown();
        }
        
        ~BallisticRuntime() {
        }
    };

    Application* CreateApplication(const std::filesystem::path& exeDir) {
        Config::Init(exeDir);
        return new BallisticRuntime();
    }
}