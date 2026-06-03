#include "file_dialog.h"

std::string FileDialog(const char* title, const char* defaultPath, bool pickFolder, const std::vector<std::pair<std::wstring, std::wstring>>& filters)
{
    std::string result;

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    IFileOpenDialog* pfd = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd)))) {
        
        if (title) {
            std::wstring wtitle(title, title + strlen(title));
            pfd->SetTitle(wtitle.c_str());
        }

        DWORD options;
        pfd->GetOptions(&options);
        if (pickFolder) options |= FOS_PICKFOLDERS;
        options |= FOS_FORCEFILESYSTEM;
        pfd->SetOptions(options);

        if (!pickFolder && !filters.empty()) {
            std::vector<COMDLG_FILTERSPEC> specs;
            for (auto& f : filters)
                specs.push_back({ f.first.c_str(), f.second.c_str() });
            pfd->SetFileTypes((UINT)specs.size(), specs.data());
        }

        if (defaultPath && std::filesystem::exists(defaultPath)) {
            std::wstring wpath(defaultPath, defaultPath + strlen(defaultPath));
            IShellItem* psi = nullptr;
            if (SUCCEEDED(SHCreateItemFromParsingName(wpath.c_str(), nullptr, IID_PPV_ARGS(&psi)))) {
                pfd->SetFolder(psi);
                psi->Release();
            }
        }

        if (SUCCEEDED(pfd->Show(nullptr))) {
            IShellItem* psi = nullptr;
            if (SUCCEEDED(pfd->GetResult(&psi))) {
                PWSTR path = nullptr;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                    result = std::filesystem::path(path).string();
                    CoTaskMemFree(path);
                }
                psi->Release();
            }
        }

        pfd->Release();
    }

    CoUninitialize();
    return result;
}
