#pragma once
#include <Ballistic.h>
#include "Panels/IPanel.h"

namespace Ballistic {

	class ViewportPanel : public IPanel {
	public:
        ViewportPanel(std::shared_ptr<Renderer> renderer);
        
		void Init() override;
		void OnImGuiRender() override;
		void OnEvent(void* ePtr) override;

    private:
        std::shared_ptr<Renderer> m_renderer;

        bool wasImguiInput;
        ImVec2 prevViewportSize = ImVec2(0.0f, 0.0f);
        ImVec2 viewportSize, topLeftTextureCoords, bottomRightTextureCoords;

        float ASPECT = 16.0f / 9.0f;
	};
}