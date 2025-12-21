#include "MenuBar.h"

#include "tinyfiledialogs.h"

namespace Ballistic {

    MenuBar::MenuBar(std::shared_ptr<ProjectManager> projectManager, std::shared_ptr<Window> window) {
        m_projectManager = projectManager;
        m_window = window;
    }
	
	void MenuBar::Init() {
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

	namespace fs = std::filesystem;

void selectFolderAndPrintTree() {
    const char* folderPath = tinyfd_selectFolderDialog("Select a folder", nullptr);
    if (!folderPath) {
        std::cout << "No folder selected.\n";
        return;
    }

    fs::path root(folderPath);
    if (!fs::exists(root) || !fs::is_directory(root)) {
        std::cerr << "Selected path is not a valid folder.\n";
        return;
    }

    std::cout << "Folder tree of: " << root << "\n";

    std::function<void(const fs::path&, int)> printTree;
    printTree = [&](const fs::path& path, int indent) {
        for (const auto& entry : fs::directory_iterator(path)) {
            std::string name = entry.path().filename().string();

            // Skip unwanted directories entirely
            if (entry.is_directory() && (name == "build" || name == "external" || name == ".git")) continue;

            for (int i = 0; i < indent; ++i) std::cout << "  "; // indent
            std::cout << (entry.is_directory() ? "[D] " : "[F] ") << name << "\n";

            // Only recurse into directories
            if (entry.is_directory()) {
                printTree(entry.path(), indent + 1);
            } else {
                // Skip files inside external subfolders
                if (entry.path().parent_path().string().find("external/") != std::string::npos) continue;
            }
        }
    };

    printTree(root, 0);
}


	void MenuBar::OnImGuiRender() {
		// entt::entity& selected = editorState.temp.selectedEntity;

		if (ImGui::BeginMainMenuBar()) {

			if (ImGui::BeginMenu("Project")) {

				if (ImGui::MenuItem("New Scene")) {
				}
				if (ImGui::MenuItem("Open Scene")) {

					const char* folderPath = tinyfd_saveFileDialog(
						"Save your file", "C:\\Users\\Marcus\\Documents\\New Project", 0, nullptr, nullptr
					);
					// if (folderPath)
						// m_ProjectManager->NewProject(std::filesystem::path(folderPath));

				}
				if (ImGui::MenuItem("Open Recent")) { selectFolderAndPrintTree(); }
				
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

				if (ImGui::MenuItem(m_window->GetState().fullscreen ? "Windowed" : "Fullscreen")) m_window->ToggleFullscreen(!m_window->GetState().fullscreen);

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
	
	void MenuBar::OnEvent(void* e) {
	}
}