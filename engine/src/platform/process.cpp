#include "process.h"

bool LaunchProcess(const std::filesystem::path& exePath, const std::filesystem::path& workingDir, const std::wstring& args)
{
    if (!std::filesystem::exists(exePath)) return false;

    std::wstring cmd = L"\"" + exePath.wstring() + L"\"";
    if (!args.empty()) cmd += L" " + args;

    std::vector<wchar_t> mutableCmd(cmd.begin(), cmd.end());
    mutableCmd.push_back(L'\0');

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};
    BOOL ok = CreateProcessW(exePath.wstring().c_str(), mutableCmd.data(), nullptr, nullptr, FALSE, 0, nullptr, workingDir.wstring().c_str(), &si, &pi);

    if (!ok) return false;
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}
