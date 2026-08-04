#include <core/assets/asset_common.h>
#include <toml++/toml.hpp>

namespace ballistic {

AssetInfo read_asset_info(const std::filesystem::path& p_path)
{
    AssetInfo info;

    toml::table tbl;
    try { tbl = toml::parse_file(p_path.string()); }
    catch (const toml::parse_error&) { return info; }

    auto asset = tbl["asset"];
    info.version = static_cast<uint32_t>(asset["version"].value_or<int64_t>(0));
    if (auto g = asset["guid"].value<std::string>()) info.guid = Guid::from_string(*g);
    info.type = asset_type_from_u32(static_cast<uint32_t>(asset["type"].value_or<int64_t>(0)));

    return info;
}

}