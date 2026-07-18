#include <core/project/project.h>
#include <core/log/log.h>
#include <core/io/path.h>
#include <fstream>

namespace ballistic {

Error Project::load(const std::filesystem::path& p_root)
{
    using enum Error;
    unload();

    std::filesystem::path file = p_root / FILE_NAME;
    std::ifstream f(file);
    if (!f) { log_write("Project: no %s in %s", FILE_NAME, p_root.string().c_str()); return Failed; }

    root = p_root;
    name = p_root.filename().string();

    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;

        size_t sp = line.find(' ');
        if (sp == std::string::npos) continue;
        std::string key = line.substr(0, sp);
        std::string value = line.substr(sp + 1);

        if (key == "project.name") { name = value; continue; }
    }

    loaded = true;
    log_write("Project loaded: %s (%s)", name.c_str(), root.string().c_str());
    return Ok;
}

Error Project::save() const
{
    using enum Error;
    if (root.empty()) return Failed;

    std::ofstream f(root / FILE_NAME);
    if (!f) return Failed;

    f << "project.name " << name << '\n';
    return Ok;
}

Error Project::create(const std::filesystem::path& p_root, std::string_view p_name)
{
    using enum Error;

    std::error_code ec;
    std::filesystem::create_directories(p_root, ec);
    std::filesystem::create_directories(p_root / ".ballistic", ec);
    if (ec) return Failed;

    Paths::set_hidden(p_root / ".ballistic");

    Project p;
    p.root = p_root;
    p.name = p_name;
    return p.save();
}

void Project::unload()
{
    root.clear();
    name.clear();
    loaded = false;
}

// std::filesystem::path Project::resolve(std::string_view p_res) const
// {
//     constexpr std::string_view PREFIX = "res://";
//     if (p_res.size() >= PREFIX.size() && p_res.substr(0, PREFIX.size()) == PREFIX)
//         p_res.remove_prefix(PREFIX.size());
//     return root / p_res;
// }

// std::string Project::to_res_path(const std::filesystem::path& p_absolute) const
// {
//     std::error_code ec;
//     std::filesystem::path rel = std::filesystem::relative(p_absolute, root, ec);
//     if (ec || rel.empty()) return {};
//
//     std::string s = rel.generic_string();
//     return "res://" + s;
// }

}