#include "workspace.h"

void EditorWorkspace::Load()
{
    PWSTR rawRoaming = nullptr;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &rawRoaming);
    roamingRoot = std::filesystem::path(rawRoaming) / "Ballistic" / "BallisticEngine";
    CoTaskMemFree(rawRoaming);
    std::filesystem::create_directories(roamingRoot);

    PWSTR rawLocal = nullptr;
    SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &rawLocal);
    localRoot = std::filesystem::path(rawLocal) / "Ballistic" / "BallisticEngine";
    CoTaskMemFree(rawLocal);
    std::filesystem::create_directories(localRoot);

    LOG_DEBUG("Roaming AppData: %s", roamingRoot.string().c_str());
    LOG_DEBUG("Local AppData: %s", localRoot.string().c_str());
 
    registry.Load(roamingRoot);
 
    std::filesystem::path cfgPath = roamingRoot / "editor.cfg";
    if (std::filesystem::exists(cfgPath)) {
        try {
            auto cfg = toml::parse_file(cfgPath.string());
            config.autosaveEnabled  = cfg["autosave_enabled"].value_or(true);
            config.autosaveInterval = (float)cfg["autosave_interval"].value_or<double>(120.0);
        } catch (const toml::parse_error& e) {
            LOG_ERROR("Failed to parse editor.cfg: %s", e.what());
        }
    }
}

void EditorWorkspace::Save()
{
    registry.Save();
    try {
        toml::table cfg;
        cfg.insert("autosave_enabled",  config.autosaveEnabled);
        cfg.insert("autosave_interval", (double)config.autosaveInterval);
        std::ofstream f(roamingRoot / "editor.cfg");
        f << cfg;
    } catch (...) {
        LOG_ERROR("Failed to save editor.cfg");
    }
}

void EditorWorkspace::TouchProject(const std::filesystem::path& projectPath)
{
    registry.UpdateTimestamp(projectPath);
    registry.Save();
}
