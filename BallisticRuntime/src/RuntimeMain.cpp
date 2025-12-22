#include <Ballistic.h>
#include <Entry/BallisticEntrypoint.h>
#include "RuntimeLayer.h"

namespace Ballistic {

    class BallisticRuntime : public Application {
    public:
        BallisticRuntime()
            : Application(WindowPresets::Runtime()) {

            m_layerStack->PushLayer(std::make_shared<RuntimeLayer>(GetAppContext(), std::string("RuntimeLayer")));
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