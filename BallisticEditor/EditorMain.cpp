#include <Ballistic.h>
#include <Entry/Entrypoint.h>

namespace ballistic
{
    class EditorApplication : public IApplication
    {
    public:
        bool Init() override {
            std::cout << "Editor initialized\n";

            WindowSettings editorSettings;
            m_window = std::make_unique<Window>();
            if (!m_window->Init(editorSettings)) {
                return false;
            }

            return true;
        }

        void Update(float deltaTime) override {
            m_window->Update();

            if (m_window->ShouldClose())
                GetRoot()->RequestShutdown();
        }

        void Shutdown() override {
            m_window->Shutdown();
                
            std::cout << "Editor shutdown\n";
        }

    private:
        std::unique_ptr<Window> m_window;
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