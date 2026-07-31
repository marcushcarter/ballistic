#include <drivers/windows/dialogs_win32.h>
#include <windows.h>
#include <commdlg.h>
#include <shobjidl.h>

namespace ballistic::drivers {

std::wstring save_file_dialog_win32(const wchar_t* p_filter, const wchar_t* p_default_ext)
{
    wchar_t path[MAX_PATH] = {};

    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFilter = p_filter;
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = p_default_ext;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (GetSaveFileNameW(&ofn)) {
        return std::wstring(path);
    }

    return {};
}

std::wstring open_folder_dialog_win32(const wchar_t* p_title)
{
    std::wstring result;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool needs_uninit = SUCCEEDED(hr);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        return result;

    IFileOpenDialog* dialog = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
    if (SUCCEEDED(hr)) {
        DWORD opts = 0;
        dialog->GetOptions(&opts);
        dialog->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
        if (p_title) dialog->SetTitle(p_title);
        dialog->SetOkButtonLabel(L"Select Folder");

        if (dialog->Show(GetActiveWindow()) == S_OK) {
            IShellItem* item = nullptr;
            if (SUCCEEDED(dialog->GetResult(&item))) {
                PWSTR path = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                    result.assign(path);
                    CoTaskMemFree(path);
                }
                item->Release();
            }
        }
        dialog->Release();
    }

    if (needs_uninit) CoUninitialize();
    return result;
}

}