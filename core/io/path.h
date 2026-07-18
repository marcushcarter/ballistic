#pragma once
#include <core/log/error.h>
#include <filesystem>
#include <string_view>

namespace ballistic {

struct Paths
{
    static std::filesystem::path local_data(std::wstring_view p_subpath = {});
    static std::filesystem::path local_low_data(std::wstring_view p_subpath = {});
    static std::filesystem::path roaming_data(std::wstring_view p_subpath = {});

    static std::filesystem::path shader_cache();
    static std::filesystem::path pipeline_cache();

    static std::filesystem::path screenshots();

    static std::filesystem::path executable_dir();

    static Error set_hidden(const std::filesystem::path& p_path, bool p_hidden = true);
    static void reveal_in_explorer(const std::filesystem::path& p_path);
};
    
};