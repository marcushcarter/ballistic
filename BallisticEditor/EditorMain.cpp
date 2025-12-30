#include <Ballistic.h>
#include <Entry/Entrypoint.h>
#include "ImGuiLayer.h"

namespace ballistic
{
    class EditorApplication : public IApplication
    {
    public:
        bool OnInit() override {
            m_window->SetIcon(GetResDirectory() / "Icons/favicon.png");

            auto imguiLayer = std::make_shared<ImGuiLayer>(m_layerContext);
            GetLayerStack()->PushLayer(imguiLayer);
            
            auto newScene = m_sceneManager->Create("New Scene");
            m_sceneManager->SetActiveScene(m_sceneManager->ConvertGUID(newScene.get()));
            m_sceneManager->GetActiveScene()->Create("New Node");

            LogDebug("Editor app initialized");
            return true;
        }

        void OnUpdate(float deltaTime) override {
        }

        void OnShutdown() override {
        }
    
    protected:
        WindowSettings GetWindowSettings() const override {
            WindowSettings s;
            s.width = 1280;
            s.height = 720;
            s.title = "Ballistic Editor";
            return s;
        }
    };

    Root* CreateRoot() {
        Root* root = new Root();
        root->SetApplication(std::make_unique<EditorApplication>());
        return root;
    }
    
} // namespace ballistic

int main(int argc, char** argv) {
    return ballistic::Main(argc, argv);
}