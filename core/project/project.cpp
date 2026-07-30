#include <core/project/project.h>
#include <core/io/path.h>
#include <fstream>
#include <cstdio>
#include <cstdlib>

namespace ballistic {

void Project::_resolve_dirs(const std::filesystem::path& p_root)
{
    root = p_root;
    data_dir = p_root / DIR_DATA;
    assets_dir = p_root / DIR_ASSETS;
    content_dir = p_root / DIR_CONTENT;
}

Error Project::_ensure_layout(const std::filesystem::path& p_root)
{
    using enum Error;

    const char* dirs[] = { DIR_DATA, DIR_ASSETS, DIR_CONTENT };
    for (const char* dir : dirs) {
        std::error_code ec;
        std::filesystem::create_directories(p_root / dir, ec);
        if (ec) {
            log_write("Project: failed to create '%s' (%s)", (p_root / dir).string().c_str(), ec.message().c_str());
            return Failed;
        }
    }

    return Ok;
}

std::filesystem::path Project::content_path(Guid p_guid) const
{
    char buf[Guid::PATH_BUFFER + 4];
    p_guid.to_path_chars(buf);
    std::memcpy(buf + Guid::PATH_CHARS, ".bin", 5);
    return content_dir / buf;
}

Error Project::load(const std::filesystem::path& p_root)
{
    using enum Error;
    unload();

    std::filesystem::path file = p_root / FILE_NAME;
    std::ifstream f(file);
    if (!f) { log_write("Project: no %s in %s", FILE_NAME, p_root.string().c_str()); return Failed; }

    std::string parsed_name = p_root.filename().string();
    uint32_t parsed_version = 0;

    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;

        size_t sp = line.find(' ');
        if (sp == std::string::npos) continue;
        std::string key = line.substr(0, sp);
        std::string value = line.substr(sp + 1);

        if (key == "project.version") { parsed_version = (uint32_t)std::strtoul(value.c_str(), nullptr, 10); continue; }
        if (key == "project.name") { name = value; continue; }

        if (key == "window.width")  { std::from_chars(value.data(), value.data() + value.size(), settings.width);  continue; }
        if (key == "window.height") { std::from_chars(value.data(), value.data() + value.size(), settings.height); continue; }
    }

    if (parsed_version > FORMAT_VERSION) {
        log_write("Project: '%s' is format version %u, this build supports %u.", file.string().c_str(), parsed_version, FORMAT_VERSION);
        return Failed;
    }

    if (Error e = _ensure_layout(p_root); e != Ok) return e;

    _resolve_dirs(p_root);
    name = parsed_name;
    log_write("Project loaded: %s (%s)", name.c_str(), root.string().c_str());
    return Ok;
}

Error Project::save() const
{
    using enum Error;
    if (root.empty()) return Failed;
    
    std::error_code ec;
    std::filesystem::create_directories(root / DIR_DATA, ec);
    if (ec) return Failed;

    std::ofstream f(root / FILE_NAME);
    if (!f) return Failed;

    f << "project.version " << FORMAT_VERSION << '\n';
    f << "project.name " << name << '\n';

    f << "window.width " << settings.width << '\n';
    f << "window.height " << settings.height << '\n';

    return Ok;
}

Error Project::create(const std::filesystem::path& p_root, std::string_view p_name)
{
    using enum Error;

    std::error_code ec;
    if (std::filesystem::exists(p_root / FILE_NAME, ec)) {
        log_write("Project: '%s' already contains a project.", p_root.string().c_str());
        return Failed;
    }

    if (Error e = _ensure_layout(p_root); e != Ok) return e;

    Project p;
    p._resolve_dirs(p_root);
    p.name = p_name;
    return p.save();
}

// Error Project::destroy(const std::filesystem::path& p_root)
// {
//     using enum Error;
//     std::error_code ec;
//
//     BALLISTIC_ERR_FAIL_COND_V_MSG(!std::filesystem::exists(p_root / FILE_NAME, ec) || ec, Failed, "Not a Ballistic project; refusing to delete.");
//     std::filesystem::path target = std::filesystem::weakly_canonical(p_root, ec);
//     if (ec) return Failed;
//     BALLISTIC_ERR_FAIL_COND_V_MSG(target == target.root_path(), Failed, "Refusing to remove a filesystem root.");
//
//     uintmax_t removed = std::filesystem::remove_all(target, ec);
//     if (ec || removed == static_cast<uintmax_t>(-1)) {
//         log_write("Failed to remove '%s' (%s)", target.string().c_str(), ec.message().c_str());
//         return Failed;
//     }
//
//     log_write("Destroyed project '%s' (%llu entries)", target.string().c_str(), (unsigned long long)removed);
//     return Ok;
// }

void Project::unload()
{
    root.clear();
    name.clear();
    settings = {};
}

}