#pragma once
#include "bepch.h"

namespace Ballistic {

    class SceneManager;
    class AssetPool;

    class ProjectManager {
    public:
        ProjectManager();

        std::string projectName;

        std::filesystem::path m_ProjectRoot;
        bool m_ProjectOpen = false;

        bool NewProject(const std::filesystem::path& folderpath);
        bool OpenProject(const std::filesystem::path& projectFilePath);
        bool SaveProject();
        bool CloseProject();

        std::shared_ptr<SceneManager> GetSceneManager() { return m_sceneManager; }
        std::shared_ptr<AssetPool> GetAssetPool() { return m_assetPool; }

    private:
        std::shared_ptr<SceneManager> m_sceneManager;
        std::shared_ptr<AssetPool> m_assetPool;
    };
    
}