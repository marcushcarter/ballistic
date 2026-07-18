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

void Paths::reveal_in_explorer(const std::filesystem::path& p_path)
{
    std::filesystem::path native = p_path;
    native.make_preferred();
    std::wstring arg = L"/select,\"" + native.wstring() + L"\"";
    ShellExecuteW(nullptr, nullptr, L"explorer.exe", arg.c_str(), nullptr, SW_SHOWNORMAL);
}
    
};