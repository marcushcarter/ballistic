#pragma once
#include "bepch.h"

namespace ballistic
{
    class ProjectManager
    {
    public:
        ProjectManager() = default;
        ~ProjectManager() { Shutdown(); }

        bool Init();
        void Shutdown();

        bool CreateNewProject(const std::filesystem::path& parentPath, const std::string& name = "New Project");
        bool OpenProject(const std::filesystem::path& configPath);

        const std::string& GetName() const { return m_projectName; } 

    private:
        std::string m_projectName;
    };

} // namespace ballistic