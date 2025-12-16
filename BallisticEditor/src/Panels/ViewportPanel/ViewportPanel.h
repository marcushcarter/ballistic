#pragma once

#include <Ballistic.h>
#include "Panels/IPanel.h"

namespace Ballistic {

	class ViewportPanel : public IPanel {
	public:
        ViewportPanel(std::shared_ptr<OglRenderer> oglRenderer);
        
		void init() override;
		void OnImGuiRender() override;
		void onEvent(void* e) override;

    private:
        std::shared_ptr<OglRenderer> m_OglRenderer;

        bool wasImguiInput;
        ImVec2 prevViewportSize = ImVec2(0.0f, 0.0f);
        ImVec2 viewportSize, topLeftTextureCoords, bottomRightTextureCoords;

        float ASPECT = 16.0f / 9.0f;
	};
}