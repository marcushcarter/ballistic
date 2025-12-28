#include <Ballistic.h>
#include <Entry/Entrypoint.h>
#include "ImGuiLayer.h"

namespace ballistic
{
    class EditorApplication : public IApplication
    {
    public:
        bool OnInit() override {
            LogDebug("Editor initialized");

            WindowSettings editorSettings;
            if (!m_window->Init(editorSettings)) {
                return false;
            }
            m_window->SetIcon(GetResDirectory() / "Icons/favicon.png");

            LayerContext ctx = CreateLayerContext();
            auto imguiLayer = std::make_shared<ImGuiLayer>(ctx);
            GetLayerStack()->PushLayer(imguiLayer);

            return true;
        }

        void OnUpdate(float deltaTime) override {
            GetLayerStack()->OnUpdate(deltaTime);

            m_window->Update(deltaTime);

            if (m_window->ShouldClose())
                GetRoot()->RequestShutdown();
        }

        void OnShutdown() override {
            GetLayerStack()->OnDetach();

            m_window->Shutdown();
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