#include "Root/ProjectManager/ProjectManager.h"
#include "Root/MeshManager/MeshManager.h"
#include "Root/LogManager/Log.h"

namespace ballistic
{
    bool ProjectManager::Init() {
        LogDebug("Project manager initilized");
        return true;
    }

    void ProjectManager::Shutdown() {
        m_projectName.clear();
    }
    
    bool ProjectManager::CreateNewProject(const std::filesystem::path& parentPath, const std::string& name) {
        m_projectName = (name.empty()) ? "New Project" : name;
        m_projectRoot = parentPath / m_projectName;

        if (std::filesystem::exists(m_projectRoot)) {
            LogError("Project directory already exists: ", m_projectRoot.string());
            return false;
        }

        std::filesystem::create_directories(m_projectRoot);

        std::filesystem::create_directories(m_projectRoot / "Resources" / "Meshes");
        // std::filesystem::create_directories(m_projectRoot / "Resources" / "Textures");
        // std::filesystem::create_directories(m_projectRoot / "Resources" / "Shaders");
        // std::filesystem::create_directories(m_projectRoot / "Scenes");

        std::filesystem::path configPath = m_projectRoot / "project.config";

        nlohmann::json j;
        j["name"] = m_projectName;

        std::ofstream out(configPath);
        if (!out.is_open()) {
            LogError("Failed to create project.config");
            return false;
        }

        out << j.dump(4);
        out.close();

        LogInfo("Created project: ", m_projectName);
        return true;
    }

    bool ProjectManager::OpenProject(const std::filesystem::path& configPath) {
        if (!std::filesystem::exists(configPath)) {
            LogError("project.config does not exist: ", configPath.string());
            return false;
        }

        if (configPath.filename() != "project.config") {
            LogError("Invalid project file: ", configPath.string());
            return false;
        }

        std::ifstream in(configPath);
        if (!in.is_open()) {
            LogError("Failed to open project.config");
            return false;
        }

        nlohmann::json j;
        in >> j;
        in.close();

        if (!j.contains("name")) {
            LogError("project.config missing 'name'");
            return false;
        }

        m_projectName = j["name"].get<std::string>();

        m_projectRoot = configPath.parent_path();

        GetRoot()->GetMeshManager()->DeserializeMeshes();

        LogInfo("Opened project: ", m_projectName);
        return true;
    }

} // namespace ballistic