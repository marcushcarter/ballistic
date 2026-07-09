#pragma once
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
};
    
};