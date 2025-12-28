#include "ImGuiLayer.h"
#include "Panels/PanelStack.h"

#include "Panels/DemoPanel.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace ballistic
{
    ImGuiLayer::ImGuiLayer(const LayerContext& context, const std::string& name) 
        : ILayer(context, name) {

        m_panelStack = std::make_shared<PanelStack>();

        auto demoPanel = std::make_shared<DemoPanel>();
        m_panelStack->PushPanel(demoPanel);
    }

    void ImGuiLayer::OnAttach() {
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

    void ImGuiLayer::OnDetach() {
        if (ImGui::GetCurrentContext() && m_context.window && m_context.window->GetNativeWindow()) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    }

    void ImGuiLayer::OnUpdate(float deltaTime) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        float titleBarHeight = 30.0f;

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, titleBarHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10,0));
        ImGuiWindowFlags windowFlags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings;
        
        if (ImGui::Begin("TitleBar", nullptr, windowFlags)) {
            ImGui::Text("%s", m_context.window->GetSettings().title.c_str());
            ImGui::SameLine(150);
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    ImGui::MenuItem("New");
                    ImGui::MenuItem("Open");
                    ImGui::MenuItem("Save");
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit")) {
                    ImGui::MenuItem("Undo");
                    ImGui::MenuItem("Redo");
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            ImGui::SameLine(viewport->Size.x - 70);
            if (ImGui::Button("-")) glfwIconifyWindow(m_context.window->GetNativeWindow());
            ImGui::SameLine();
            if (ImGui::Button("x")) glfwSetWindowShouldClose(m_context.window->GetNativeWindow(), true);

            ImGui::End();
            ImGui::PopStyleVar(2);
        }

        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + titleBarHeight));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - titleBarHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
        ImGui::Begin("DockSpaceWindow", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse
        );
        ImGuiID dockspace_id = ImGui::DockSpace(ImGui::GetID("DockSpace"), ImVec2(0,0), ImGuiDockNodeFlags_None);
        ImGui::End();
        ImGui::PopStyleVar(2);

        m_panelStack->OnUpdate(deltaTime);

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

    void ImGuiLayer::OnEvent(IEvent& e) {
    }
    
} // namespace ballistic
