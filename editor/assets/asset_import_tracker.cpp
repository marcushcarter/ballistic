#include <editor/assets/asset_import_tracker.h>
#include <core/project/project.h>

namespace ballistic {

ImportProgress AssetImportTracker::add(const std::filesystem::path& p_dst)
{
    auto cell = std::make_shared<std::atomic<float>>(0.0f);
    pending[p_dst] = cell;
    return cell;
}

float AssetImportTracker::progress(const std::filesystem::path& p_dst)
{
    auto it = pending.find(p_dst);
    return it == pending.end() ? -1.0f : it->second->load(std::memory_order_relaxed);

}

void AssetImportTracker::tick()
{
    std::error_code ec;
    for (auto it = pending.begin(); it != pending.end(); ) {
        const float v = it->second->load(std::memory_order_relaxed);
        if (v >= 1.0f && std::filesystem::exists(it->first, ec)) it = pending.erase(it);
        else it++;
    }
}

std::vector<std::filesystem::path> AssetImportTracker::pending_out(const std::filesystem::path& p_folder) const
{
    std::vector<std::filesystem::path> out;
    std::error_code ec;
    for (const auto& [path, cell] : pending) {
        if (path.parent_path() != p_folder) continue;
        if (std::filesystem::exists(path, ec)) continue;
        out.push_back(path);
    }
    return out;
}

Error AssetImportTracker::resolve_import(const Project& p_project, AssetType p_type, const std::filesystem::path& p_dest, Guid& r_guid, std::filesystem::path& r_content_bin)
{
    using enum Error;
    std::error_code ec;
    if (std::filesystem::exists(p_dest, ec)) {
        AssetInfo existing = read_asset_info(p_dest);
        if (!existing.valid() || existing.type != p_type) return Failed;
        r_guid = existing.guid;
    } else {
        r_guid = Guid::generate();
    }
    r_content_bin = p_project.content_path(r_guid);
    return Ok;
}

}