#include "MenuBar.h"

namespace Ballistic {

    MenuBar::MenuBar(std::shared_ptr<ProjectManager> projectManager, std::shared_ptr<IWindow> window) {
        m_ProjectManager = projectManager;
        m_Window = window;
    }
	
	void MenuBar::init() {
	}

	void MenuBar::OpenLink(const std::string& url) {
	#ifdef _WIN32
		ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	#elif __APPLE__
		std::string cmd = "open " + url;
		std::system(cmd.c_str());
	#else // Linux/Unix
		std::string cmd = "xdg-open " + url;
		std::system(cmd.c_str());
	#endif
	}

	void MenuBar::OnImGuiRender() {
		// entt::entity& selected = editorState.temp.selectedEntity;

		if (ImGui::BeginMainMenuBar()) {

			if (ImGui::BeginMenu("Project")) {

				if (ImGui::MenuItem("New Scene")) {
					// manager.Clear();
					// scene.clear();
					// selected = entt::null;
				}
				if (ImGui::MenuItem("Open Scene")) {}
				if (ImGui::MenuItem("Open Recent")) {}
				
				ImGui::Separator();

				if (ImGui::MenuItem("Save")) {}
				if (ImGui::MenuItem("Save As")) {}
				if (ImGui::MenuItem("Save Copy")) {}
				if (ImGui::MenuItem("Save Increments")) {}

				ImGui::Separator();

				if (ImGui::MenuItem("Set Project Directory")) {}
				if (ImGui::MenuItem("Import")) {

					// const char* filters[] = { "*.obj" };
					// const char* files = tinyfd_openFileDialog("Pick files", "", 1, filters, "OBJ Files", 1);

					// if (files && *files != '\0') {
					//     std::string filesStr = files;
					//     size_t start = 0, end;

					//     do {
					//         end = filesStr.find("|", start);
					//         std::string path = filesStr.substr(start, end - start);

					//         if (!path.empty()) {
					//             std::filesystem::path p(path);
					//             std::string ext = p.extension().string();
					//             std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

					//             if (ext == ".obj") {
					//                 manager.AddRtxMesh(p.stem().string(), path);
					//             }
					//         }

					//         start = (end == std::string::npos) ? filesStr.size() : end + 1;
					//     } while (start < filesStr.size());
					// }
				}

				// if (ImGui::MenuItem("Export")) {}

				ImGui::Separator();

				if (ImGui::MenuItem("Undo")) {}
				if (ImGui::MenuItem("Redo")) {}
				if (ImGui::MenuItem("Undo History")) {}

				ImGui::Separator();

				if (ImGui::MenuItem("Quit")) { glfwTerminate(); }

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Debug")) {

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Editor")) {

				if (ImGui::MenuItem("Editor Settings")) {}
				if (ImGui::MenuItem("Editor Colors")) {}

				ImGui::Separator();

				if (ImGui::MenuItem("Take Editor Screenshot")) {} // takeScreenshotNextFrame = true;
				if (ImGui::MenuItem("Take Viewport Screenshot")) {} // takeTextureScreenshotNextFrame = true;

				if (ImGui::MenuItem(m_Window->isFullscreen() ? "Windowed" : "Fullscreen")) m_Window->toggleFullscreen(!m_Window->isFullscreen());

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help")) {

				if (ImGui::MenuItem("Search Help")) {}

				ImGui::Separator();

				if (ImGui::MenuItem("Online Documentation")) OpenLink("https://ballisticstudios.ca/");
				if (ImGui::MenuItem("Forums")) OpenLink("https://ballisticstudios.ca/");
				if (ImGui::MenuItem("Community")) OpenLink("https://ballisticstudios.ca/");
				if (ImGui::MenuItem("Tutorials")) OpenLink("https://ballisticstudios.ca/");

				ImGui::Separator();

				if (ImGui::MenuItem("Report a bug")) OpenLink("https://ballisticstudios.ca/");

				ImGui::Separator();

				if (ImGui::MenuItem("About The Engine")) OpenLink("https://ballisticstudios.ca/");
				if (ImGui::MenuItem("Support Development")) OpenLink("https://ballisticstudios.ca/");

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}
	
	void MenuBar::onEvent(void* e) {
	}
}