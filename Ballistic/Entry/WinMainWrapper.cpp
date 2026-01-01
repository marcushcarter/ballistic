#ifdef _WIN32
#include <windows.h>

int main(int argc, char** argv);

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd)
{
    int argc = __argc;
    char** argv = __argv;
    return main(argc, argv);
}
#endif