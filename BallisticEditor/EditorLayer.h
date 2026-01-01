#pragma once
#include <Ballistic.h>
#include "Panels/PanelStack.h"

struct ImGuiContext;

namespace ballistic
{
    class EditorLayer : public ILayer
    {
    public:
        EditorLayer(LayerContext& context, const std::string& name = "Editor");
        ~EditorLayer() override { OnDetach(); }

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(float deltaTime) override;
        void OnEvent(IEvent& e) override;

        PanelStack* GetPanelStack() { return &m_panelStack; }

    private:
        PanelStack m_panelStack;

        void MenuBar();
    };
    
} // namespace ballistic
