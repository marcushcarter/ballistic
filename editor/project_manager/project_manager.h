#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace ballistic {

struct ProjectManager
{
    struct Entry {
        std::string name;
        std::filesystem::path path;
        bool favorite = false;
    };

    std::vector<Entry> recent;
    int selected = -1;

    bool open_requested = false;
    std::filesystem::path open_path;

    void load_recent();
    void save_recent();
    void add_recent(const std::filesystem::path& p_root, std::string_view p_name);
    void sort_entries();

    void on_update();
};

}