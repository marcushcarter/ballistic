#pragma once
#include <editor/editor_context.h>
#include <core/base/error.h>
#include <filesystem>
#include <string>
#include <vector>

namespace ballistic {

struct ProjectManager
{
    struct Entry {
        std::string name;
        std::filesystem::path path;
    };

    std::vector<Entry> recent;
    int selected = -1;

    Error initialize();
    void shutdown();

    void load_recents();
    void save_recents();
    void add_recent(const std::filesystem::path& p_root, std::string_view p_name);

    void on_update(EditorContext& ctx);
};

}