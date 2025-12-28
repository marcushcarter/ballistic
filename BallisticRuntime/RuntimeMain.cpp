#include <Ballistic.h>
#include <Entry/Entrypoint.h>

namespace ballistic
{
    class RuntimeApplication : public IApplication
    {
    public:
        bool OnInit() override {
            LogDebug("Runtime initialized");

            WindowSettings runtimeSettings = WindowSettings::LoadSettingsFromProject();
            if (!m_window->Init(runtimeSettings)) {
                return false;
            }

            return true;
        }

        void OnUpdate(float deltaTime) override {
            m_window->Update(deltaTime);

            if (m_window->ShouldClose())
                GetRoot()->RequestShutdown();
        }

        void OnShutdown() override {
            m_window->Shutdown();
        }
    };

    Root* CreateRoot() {
        Root* root = new Root();
        root->SetApplication(std::make_unique<RuntimeApplication>());
        return root;
    }
    
} // namespace ballistic

int main(int argc, char** argv) {
    return ballistic::Main(argc, argv);
}
