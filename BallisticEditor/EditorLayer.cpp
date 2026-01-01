#include "EditorLayer.h"
#include "Panels/MenuPanel.h"
#include "Panels/LauncherPanel.h"
#include "Panels/DemoPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ConsolePanel.h"
#include <imgui.h>
#include <ImGuizmo.h>

namespace ballistic
{
    EditorLayer::EditorLayer(LayerContext& context, const std::string& name) 
        : ILayer(context, name), m_panelStack(context) {
    }

    void EditorLayer::OnAttach() {
        m_panelStack.OpenLauncher(m_context);
    }

    void EditorLayer::OnDetach() {
    }

    void EditorLayer::OnUpdate(float deltaTime) {
        m_panelStack.OnUpdate(deltaTime);
    }
    
    void EditorLayer::OnEvent(IEvent& e) {
        m_panelStack.DispatchEvent(e);
    }
    
} // namespace ballistic
