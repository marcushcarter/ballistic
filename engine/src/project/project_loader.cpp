#include "project_loader.h"
#include "project.h"

void ProjectLoader::Begin(Project& project, const std::filesystem::path& path)
{
    done = false;
    failed = false;
 
    future = std::async(std::launch::async, [this, &project, path]() {
        if (!std::filesystem::exists(path)) {
            LOG_ERROR("Project path does not exist: %s", path.string().c_str());
            failed = true;
            done = true;
            return;
        }
 
        if (!project.Load(path)) {
            LOG_ERROR("Failed to deserialize project: %s", path.string().c_str());
            failed = true;
            done = true;
            return;
        }
 
        done = true;
    });
}

ProjectLoader::Status ProjectLoader::Poll() const
{
    if (!future.valid()) return Status::Idle;
    if (!done.load()) return Status::Loading;
    return failed.load() ? Status::Failed : Status::Succeeded;
}

void ProjectLoader::Reset()
{
    if (future.valid()) future.get();
    done = false;
    failed = false;
}
