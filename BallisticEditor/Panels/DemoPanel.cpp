#include "Panels/DemoPanel.h"

namespace ballistic
{
    DemoPanel::DemoPanel(LayerContext& context, const std::string& name) 
        : IPanel(context, name) {}
    
    void DemoPanel::OnAttach() {
    }
    
    void DemoPanel::OnDetach() {
    }

    void DemoPanel::OnUpdate(float deltaTime) {
        ImGui::ShowDemoWindow();
    }

    void DemoPanel::OnEvent(IEvent& e) {

    }

} // namespace ballistic
