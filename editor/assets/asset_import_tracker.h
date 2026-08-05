#pragma once
#include <core/assets/guid.h>
#include <core/assets/asset_common.h>
#include <core/base/error.h>
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <vector>

namespace ballistic {

struct Project;

using ImportProgress = std::shared_ptr<std::atomic<float>>;

struct AssetImportTracker
{
    std::unordered_map<std::filesystem::path, ImportProgress> pending;

    ImportProgress add(const std::filesystem::path& p_dst);
    float progress(const std::filesystem::path& p_dst);    
    void tick();

    std::vector<std::filesystem::path> pending_out(const std::filesystem::path& p_folder) const;

    static Error resolve_import(const Project& p_project, AssetType p_type, const std::filesystem::path& p_dest, Guid& r_guid, std::filesystem::path& r_content_bin);
};
    
}