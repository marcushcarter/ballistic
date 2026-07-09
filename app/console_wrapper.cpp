#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    std::wstring path(exePath);
    size_t lastSlash = path.find_last_of(L"\\/");
    std::wstring targetExe = path.substr(0, lastSlash + 1) + L"ballistic_editor_app.exe";

    HANDLE jobHandle = CreateJobObjectW(nullptr, nullptr);
    if (!jobHandle) {
        wprintf(L"CreateJobObject failed: %lu\n", GetLastError());
        return -1;
    }

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli{};
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    SetInformationJobObject(jobHandle, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    std::wstring cmdLine = L"\"" + targetExe + L"\"";
    std::vector<wchar_t> cmdBuf(cmdLine.begin(), cmdLine.end());
    cmdBuf.push_back(L'\0');

    if (!CreateProcessW(targetExe.c_str(), cmdBuf.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi)) {
        wprintf(L"CreateProcess failed: %lu\n", GetLastError());
        CloseHandle(jobHandle);
        return -1;
    }

    AllowSetForegroundWindow(pi.dwProcessId);

    AssignProcessToJobObject(jobHandle, pi.hProcess);
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(jobHandle);

    return static_cast<int>(exitCode);
}