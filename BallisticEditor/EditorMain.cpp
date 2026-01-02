#include <Ballistic.h>
#include <Entry/Entrypoint.h>
#include "ImGuiLayer.h"
#include "EditorLayer.h"

namespace ballistic
{
    class EditorApplication : public IApplication
    {
    public:
        bool OnInit() override {
            m_window->SetIcon(GetResDirectory() / "Icons/favicon.png");

            m_imguiLayer = std::make_unique<ImGuiLayer>(m_layerContext);
            m_imguiLayer->Init();
            
            auto editorLayer = std::make_shared<EditorLayer>(m_layerContext);
            m_layerContext.layerStack->PushLayer(editorLayer);
            
            // Test Dragon Model in Scene
            auto newScene = m_sceneManager->Create("New Scene");
            m_sceneManager->SetActiveScene(m_sceneManager->ConvertGUID(newScene.get()));
            auto node = m_sceneManager->GetActiveScene()->Create("Stanford Dragon");
            EntityHandle e(node, m_sceneManager->GetActiveScene()->GetRegistry());
            auto& tfmComp = e.add<TransformComponent>();
            tfmComp.position = glm::vec3(0, -4, 0);
            tfmComp.scale = glm::vec3(0.1);
            auto& meshComp = e.add<MeshComponent>();
            // meshComp.mesh = GetRoot()->GetMeshManager()->GetMeshGUIDByName("defaultMaterial");

            LogDebug("Editor app initialized");
            return true;
        }

        void BeginFrame() override {
            if (m_imguiLayer) m_imguiLayer->BeginFrame();
        }

        void OnUpdate(float deltaTime) override {
        }

        void EndFrame() override {
            if (m_imguiLayer) m_imguiLayer->EndFrame();
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

    private:
        std::unique_ptr<ImGuiLayer> m_imguiLayer; 
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