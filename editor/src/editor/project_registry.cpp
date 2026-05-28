#include "project_registry.h"
#include "core/timestamp.h"

void ProjectRegistry::Load(const std::filesystem::path& roamingRoot)
{
    (void)roamingRoot;
    // TODO: move from ProjectManager::Load

}

void ProjectRegistry::Save()
{
    // TODO: move from ProjectManager::Save

}

ProjectEntry* ProjectRegistry::Find(const std::filesystem::path& projectPath)
{
    (void)projectPath;
    return nullptr;

}

void ProjectRegistry::Add(const ProjectEntry& entry)
{
    (void)entry;
}

void ProjectRegistry::Remove(size_t index)
{
    (void)index;
}

void ProjectRegistry::UpdateTimestamp(const std::filesystem::path& projectPath)
{
    // TODO: use FormatTimestampNow()
    (void)projectPath;
}
