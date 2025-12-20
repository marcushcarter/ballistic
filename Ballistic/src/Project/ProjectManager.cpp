#include "ProjectManager.h"
#include "Scene/SceneManager.h"
#include "Assets/AssetPool.h"

namespace Ballistic {

    ProjectManager::ProjectManager() {
        m_sceneManager = std::make_shared<SceneManager>();
        m_sceneManager->createScene("New Scene");
    }

    bool ProjectManager::NewProject(const std::filesystem::path& folderpath) {
        // if (std::filesystem::exists(folderpath))
        //     return false;

        // std::filesystem::create_directories(folderpath / "Scenes");
        // std::filesystem::create_directories(folderpath / "Assets");

        // json settings;
        // settings["project_name"] = folderpath.filename().string();
        // settings["version"] = 1;
        // settings["start_scene"] = "Scenes/MainScene.scene";

        // std::ofstream out(folderpath / "ProjectSettings.json");
        // out << settings.dump(4);
        // out.close();

        // m_ProjectRoot = folderpath;
        // m_ProjectOpen = true;

        // m_SceneManager = std::make_shared<SceneManager>();
        // m_SceneManager->createScene("New Scene");

        // return true;

        return false;
    }

    bool ProjectManager::OpenProject(const std::filesystem::path& projectFilePath) {
        return false;
    }
    
    bool ProjectManager::SaveProject() {
        return false;
    }
    
    bool ProjectManager::CloseProject() {
        return false;
    }
    
}