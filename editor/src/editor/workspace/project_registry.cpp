#include <editor/workspace/project_registry.h>
#include <core/timestamp.h>
#include <core/log.h>
#include <toml++/toml.hpp>
#include <fstream>

void ProjectRegistry::Load(const std::filesystem::path& roamingRoot)
{
    cfgPath = roamingRoot / "projects.cfg";
    entries.clear();
    if (!std::filesystem::exists(cfgPath)) return;

    try {
        auto root = toml::parse_file(cfgPath.string());
        auto arr = root["projects"].as_array();
        if (!arr) return;

        arr->for_each([&](auto& el) {
            if (!el.is_table()) return;
            auto& t = *el.as_table();

            ProjectEntry p;
            p.path = t["path"].value_or<std::string>("");
            p.favorite = t["favorite"].value_or(false);
            p.lastOpened = t["last_opened"].value_or<std::string>("");
            if (p.path.empty()) return;

            std::filesystem::path blstPath = std::filesystem::path(p.path) / "project.blst";
            if (std::filesystem::exists(blstPath)) {
                try {
                    auto blst = toml::parse_file(blstPath.string());
                    p.name = blst["name"].value_or<std::string>(std::filesystem::path(p.path).filename().string());
                    p.engineVersion = blst["engine_version"].value_or<std::string>("");
                } catch (...) {
                    p.name = std::filesystem::path(p.path).filename().string();
                }
            } else {
                p.name = std::filesystem::path(p.path).filename().string();
            }

            entries.push_back(p);
        });

    } catch (const toml::parse_error& e) {
        (void)e;
        LOG_ERROR("Failed to parse projects.cfg: %s", e.what());
    }
}

void ProjectRegistry::Save()
{
    if (cfgPath.empty()) return;

    toml::table root;
    toml::array arr;

    for (auto& p : entries) {
        toml::table entry;
        entry.insert("path", p.path);
        entry.insert("favorite", p.favorite);
        entry.insert("last_opened", p.lastOpened);
        arr.push_back(entry);
    }

    root.insert("projects", arr);
    std::ofstream file(cfgPath);
    file << root;
}

ProjectEntry* ProjectRegistry::Find(const std::filesystem::path& projectPath)
{
    for (auto& p : entries) {
        if (std::filesystem::path(p.path) == projectPath) return &p;
    }
    return nullptr;
}

void ProjectRegistry::Add(const ProjectEntry& entry)
{
    entries.push_back(entry);
}

void ProjectRegistry::Remove(size_t index)
{
    if (index >= entries.size()) return;
    entries.erase(entries.begin() + index);
}

void ProjectRegistry::UpdateTimestamp(const std::filesystem::path& projectPath)
{
    if (auto* p = Find(projectPath)) {
        p->lastOpened = FormatTimestampNow();
    }
}
