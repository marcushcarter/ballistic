#include "project.h"

bool Project::Load(const std::filesystem::path& projectPath)
{
    if (!std::filesystem::exists(projectPath / "project.blst")) {
        LOG_ERROR("project.blst not found: %s", projectPath.string().c_str());
        return false;
    }

    try {
        auto blst = toml::parse_file((projectPath / "project.blst").string());
        name = blst["name"].value_or<std::string>("");
        engineVersion = blst["engine_version"].value_or<std::string>("");
        path = projectPath;
    } catch (const toml::parse_error& e) {
        LOG_ERROR("Failed to parse project.blst: %s", e.what());
        return false;
        }


    LOG_INFO("Editing project: %s (%s)", name.c_str(), path.string().c_str());
    return true;
}

bool Project::Save()
{
    return true;
}

void Project::Close()
{
    name.clear();
    engineVersion.clear();
    path.clear();
    LOG_DEBUG("Project closed");
}
