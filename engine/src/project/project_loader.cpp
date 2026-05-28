#include "project_loader.h"

void ProjectLoader::Begin(Project& project, const std::filesystem::path& path)
{
    // TODO: move async lambda from Application::OpenProject
    (void)project;
    (void)path;
}

bool Poll()
{
    // TODO: returns true when loading has terminated
    return false;
}
