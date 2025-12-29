#pragma once
#include "Panels/IPanel.h"
#include "imgui.h"

namespace ballistic
{
	class ViewportPanel : public IPanel {
	public:
        ViewportPanel(LayerContext& context, const std::string& name = "Viewport");
        ~ViewportPanel() override { OnDetach(); }
        
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float deltaTime) override;
		void OnEvent(IEvent& e) override;
    
    private:
        bool wasImguiInput;
        ImVec2 prevViewportSize = ImVec2(0.0f, 0.0f);
        ImVec2 viewportSize, topLeftTextureCoords, bottomRightTextureCoords;

        float ASPECT = 16.0f / 9.0f;
	};
}