#include <core/io/path.h>
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>

namespace ballistic {

static std::filesystem::path _known_folder(const KNOWNFOLDERID& p_id, std::wstring_view p_subpath)
{
    PWSTR raw = nullptr;
    if (FAILED(SHGetKnownFolderPath(p_id, 0, nullptr, &raw)) || !raw) {
        if (raw) CoTaskMemFree(raw);
        return {};
    }
    std::filesystem::path dir = raw;
    CoTaskMemFree(raw);

    dir /= L"Ballistic Games/Ballistic";
    if (!p_subpath.empty()) dir /= p_subpath;

    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir;
}
    
std::filesystem::path Paths::local_data(std::wstring_view p_subpath) { return _known_folder(FOLDERID_LocalAppData, p_subpath); }
std::filesystem::path Paths::local_low_data(std::wstring_view p_subpath) { return _known_folder(FOLDERID_LocalAppDataLow, p_subpath); }
std::filesystem::path Paths::roaming_data(std::wstring_view p_subpath) { return _known_folder(FOLDERID_RoamingAppData, p_subpath); }

std::filesystem::path Paths::shader_cache() { return local_data(L"shader_cache"); }
std::filesystem::path Paths::pipeline_cache() { return local_data(L"pipeline_cache"); }

std::filesystem::path Paths::screenshots() { return roaming_data(L"screenshots"); }

std::filesystem::path Paths::executable_dir()
{
    wchar_t buf[MAX_PATH]{};
    DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (n == 0 || n == MAX_PATH) return {};
    return std::filesystem::path(buf).parent_path();
}

Error Paths::set_hidden(const std::filesystem::path& p_path, bool p_hidden)
{
    using enum Error;
    DWORD attrs = GetFileAttributesW(p_path.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) return Failed;
    DWORD next = p_hidden ? (attrs | FILE_ATTRIBUTE_HIDDEN) : (attrs & ~FILE_ATTRIBUTE_HIDDEN);
    if (next == attrs) return Ok;
    return SetFileAttributesW(p_path.c_str(), next) ? Ok : Failed;
}

void Paths::reveal_in_explorer(const std::filesystem::path& p_path)
{
    std::filesystem::path native = p_path;
    native.make_preferred();

    if (std::filesystem::is_directory(p_path)) {
        ShellExecuteW(nullptr, L"open", native.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    } else {
        std::wstring arg = L"/select,\"" + native.wstring() + L"\"";
        ShellExecuteW(nullptr, nullptr, L"explorer.exe", arg.c_str(), nullptr, SW_SHOWNORMAL);
    }
}

void Paths::asset_move(const std::filesystem::path& src, const std::filesystem::path& dst_dir)
{
    if (src.empty() || dst_dir.empty()) return;
    if (src.parent_path() == dst_dir) return;

    auto s = src.begin();
    auto d = dst_dir.begin();
    bool src_is_prefix = true;
    for (; s != src.end(); ++s, ++d) {
        if (d == dst_dir.end() || *d != *s) { src_is_prefix = false; break; }
    }
    if (src_is_prefix) return;

    const std::filesystem::path dst = dst_dir / src.filename();
    if (std::filesystem::exists(dst)) return;

    std::error_code ec;
    std::filesystem::rename(src, dst, ec);
}

bool Paths::is_under(const std::filesystem::path& p, const std::filesystem::path& base)
{
    auto pb = p.begin();
    auto bb = base.begin();
    for (; bb != base.end(); ++bb, ++pb) if (pb == p.end() || *pb != *bb) return false;
    return true;
}

};