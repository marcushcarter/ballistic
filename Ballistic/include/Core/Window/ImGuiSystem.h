#pragma once
#include "bepch.h"

namespace Ballistic {

    class ImGuiSystem {
    public:
        ImGuiSystem() = default;
		~ImGuiSystem();

		void Init(GLFWwindow* window, bool viewports = false, bool dockspace = false);
		void BeginFrame();
		void EndFrame();
		void Shutdown();

		ImGuiContext* GetContext() const { return m_context; }

	private:
		ImGuiContext* m_context = nullptr;
		bool m_dockspace = false;
		bool m_viewports = false;
    };

}