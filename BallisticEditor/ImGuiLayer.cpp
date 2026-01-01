#include "ImGuiLayer.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <ImGuizmo.h>

namespace ballistic
{
    ImGuiLayer::ImGuiLayer(LayerContext& context) : m_context(context) {
    }

    void ImGuiLayer::Init() {        
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
		io.Fonts->AddFontFromFileTTF((IApplication::GetResDirectory() / "Fonts/Noto_Sans/static/NotoSans-Regular.ttf").string().c_str(), 16.0f, &cfg);
		
		cfg.MergeMode = true;
		static const ImWchar icons_ranges[] = { 0xF000, 0xF8FF, 0 };
		io.Fonts->AddFontFromFileTTF((IApplication::GetResDirectory() / "Fonts/fontawesome-free-6.6.0-desktop/Font Awesome 6 Free-Solid-900.otf").string().c_str(), 13.0f, &cfg, icons_ranges);
		
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

        ImGui_ImplGlfw_InitForOpenGL(m_context.window->GetNativeWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    void ImGuiLayer::Shutdown() {
        if (ImGui::GetCurrentContext() && m_context.window && m_context.window->GetNativeWindow()) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    }
    
    void ImGuiLayer::BeginFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuizmo::BeginFrame();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
            
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpaceMain", nullptr, window_flags);
        
        ImGuiID dockspace_id = ImGui::GetID("DockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        
        ImGui::End();
        ImGui::PopStyleVar(2);
    }

    void ImGuiLayer::EndFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        auto io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup);
		}
    }
    
} // namespace ballistic
