#include <windows.h>
#include <string>

int main()
{
    char dir[MAX_PATH];
    GetModuleFileNameA(NULL, dir, MAX_PATH);

    std::string path(dir);
    path = path.substr(0, path.find_last_of("\\/") + 1);
    path += "Ballistic_v" BALLISTIC_VERSION_STR "_win64.exe";

    LPSTR cmdLine = GetCommandLineA();

    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);

    CreateProcessA(path.c_str(), cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (int)exitCode;
}