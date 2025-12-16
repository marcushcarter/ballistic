#pragma once

#include <Ballistic.h>

namespace Ballistic {

    class ImGuiContext {
    public:
        ImGuiContext(std::shared_ptr<IWindow> window);
		virtual ~ImGuiContext();

		void Init();
		void BeginFrame();
		void EndFrame();
		void Shutdown();

		void LoadDefaultLayout();

	private:
		std::shared_ptr<IWindow> m_Window;
    };

}