#pragma once
#include <core/log/error.h>
#include <filesystem>
#include <string>

namespace ballistic {

struct Project
{
    static constexpr uint32_t FORMAT_VERSION = 1;
    static constexpr const char* FILE_NAME = "project.ballistic";

    std::filesystem::path root;
    std::string name;
    bool loaded = false;

    Error load(const std::filesystem::path& p_root);
    Error save() const;
    void unload();

    static Error create(const std::filesystem::path& p_root, std::string_view p_name);

    // std::filesystem::path resolve(std::string_view p_res) const;
    // std::string to_res_path(const std::filesystem::path& p_absolute) const;
};

}