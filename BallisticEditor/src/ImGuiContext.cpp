#include "ImGuiContext.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "IconsFontAwesome5.h"

namespace Ballistic {
    
    ImGuiContext::ImGuiContext(std::shared_ptr<IWindow> window) {
        m_Window = window;
    }

    ImGuiContext::~ImGuiContext() {
	    ImGui_ImplOpenGL3_Shutdown();
    	switch (WindowAPI::GetAPI()) {
	    	case WindowAPI::API::GLFW:
	    		ImGui_ImplGlfw_Shutdown();
	    		break;
	    	default:
	    		break;
    	}
    	ImGui::DestroyContext();
    }

    void ImGuiContext::Init() {
		IMGUI_CHECKVERSION();
	    ImGui::CreateContext();

	    ImGuiIO& io = ImGui::GetIO();
	    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImFontConfig cfg;
		cfg.PixelSnapH = true;
		cfg.MergeMode = false;
		/* ImFont* baseFont = */io.Fonts->AddFontFromFileTTF((Config::EDITOR_RES_PATH / "Fonts/Noto_Sans/static/NotoSans-Regular.ttf").string().c_str(), 16.0f, &cfg);
		
		cfg.MergeMode = true;
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		io.Fonts->AddFontFromFileTTF((Config::EDITOR_RES_PATH / "Fonts/fontawesome-free-6.6.0-desktop/Font Awesome 6 Free-Solid-900.otf").string().c_str(), 13.0f, &cfg, icons_ranges);
		
    	ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            // style.WindowRounding = 0.0f;
            // style.TabRounding = 0.0f;
            // style.TabBarBorderSize = 0.0f;
            // style.GrabRounding = 2.0f;
            // style.ScrollbarRounding = 2.0f;
            // style.DockingSeparatorSize = 0.0f;
            // style.WindowBorderSize = 0.0f;
        }

        // style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        // style.WindowMenuButtonPosition = ImGuiDir_None;

    	switch (WindowAPI::GetAPI()) {
	    	case WindowAPI::API::GLFW:
	    		ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(m_Window->get()), true);
	    		break;

	    	default:
	    		break;
    	}
	    ImGui_ImplOpenGL3_Init("#version 460");
    }
    
    void ImGuiContext::BeginFrame() {
	    ImGui_ImplOpenGL3_NewFrame();

    	switch (WindowAPI::GetAPI()) {
	    	case WindowAPI::API::GLFW:
	    		ImGui_ImplGlfw_NewFrame();
	    		break;
	    	default:
	    		break;
    	}

    	ImGui::NewFrame();
    }

    void ImGuiContext::EndFrame() {
    	ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    	auto io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		    if (WindowAPI::GetAPI() == WindowAPI::API::GLFW) {
		        GLFWwindow* backup = static_cast<GLFWwindow*>(m_Window->get());
		        ImGui::UpdatePlatformWindows();
		        ImGui::RenderPlatformWindowsDefault();
		        glfwMakeContextCurrent(backup);
		    }
		}
    }

    void ImGuiContext::Shutdown() {
    }
    
}