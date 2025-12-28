#pragma once
#include <Ballistic.h>

struct ImGuiContext;

namespace ballistic
{
    class PanelStack;

    class ImGuiLayer : public ILayer
    {
    public:
        ImGuiLayer(LayerContext& context, const std::string& name = "ImGuiLayer");
        ~ImGuiLayer() override { OnDetach(); }

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(float deltaTime) override;
        void OnEvent(IEvent& e) override;

        std::shared_ptr<PanelStack> GetPanelStack() { return m_panelStack; }

    private:
        std::shared_ptr<PanelStack> m_panelStack;

        void MenuBar();
    };
    
} // namespace ballistic
