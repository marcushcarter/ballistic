#include <Ballistic.h>
#include <Entry/Entrypoint.h>

namespace ballistic
{
    class RuntimeApplication : public IApplication
    {
    public:
        bool OnInit() override {
            LogDebug("Runtime app initialized");
            return true;
        }

        void OnUpdate(float deltaTime) override {
            auto windowState = m_window->GetState();
            m_renderer->RequestResize(glm::vec2(windowState.width, windowState.height));
            // resize camera

            m_renderer->GetDevice()->BlitToScreen();
        }

        void OnShutdown() override {
        }
    
    protected:
        WindowSettings GetWindowSettings() const override {
            return WindowSettings::LoadSettingsFromProject();
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
