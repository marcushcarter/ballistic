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
        
        void BeginFrame() override {}

        void OnUpdate(float deltaTime) override {
            auto windowState = m_window->GetState();
            m_renderer->RequestResize(glm::vec2(windowState.width, windowState.height));

            auto* scene = m_sceneManager->GetActiveScene();
            if (scene->GetMainCamera() != entt::null) {
                EntityHandle e(scene->GetMainCamera(), scene->GetRegistry());
                if (e.has<TransformComponent>() && e.has<CameraComponent>()) {
                    auto& camComp = e.get<CameraComponent>();
                    auto& tfmComp = e.get<TransformComponent>();

                    camComp.OnUpdate(tfmComp);

                    m_renderer->SubmitCamera(camComp.view, camComp.projection, tfmComp.position);
                }
            }
        }

        void EndFrame() override {}

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
