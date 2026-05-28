#include "app_data.h"

AppDataPaths ResolveAppDataPaths()
{
    AppDataPaths paths;

    PWSTR rawRoaming = nullptr;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &rawRoaming);
    paths.roamingRoot = std::filesystem::path(rawRoaming) / "Ballistic" / "BallisticEngine";
    CoTaskMemFree(rawRoaming);
    std::filesystem::create_directories(paths.roamingRoot);

    PWSTR rawLocal = nullptr;
    SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &rawLocal);
    paths.localRoot = std::filesystem::path(rawLocal) / "Ballistic" / "BallisticEngine";
    CoTaskMemFree(rawLocal);
    std::filesystem::create_directories(paths.localRoot);

    LOG_DEBUG("Roaming AppData: %s", paths.roamingRoot.string().c_str());
    LOG_DEBUG("Local AppData: %s", paths.localRoot.string().c_str());

    return paths;
}