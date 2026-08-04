#pragma once
#include <core/base/error.h>
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

    static void asset_move(const std::filesystem::path& src, const std::filesystem::path& dst_dir);
    static bool is_under(const std::filesystem::path& p, const std::filesystem::path& base);
};
    
};