

#include "Entry/Entrypoint.h"
#include "Root/LogManager/Log.h"

namespace ballistic
{

    int Main(int argc, char** argv) {
        std::filesystem::path exePath = std::filesystem::weakly_canonical(argv[0]);
        std::filesystem::path exeDir = exePath.parent_path();

        extern Root* CreateRoot();
        Root* root = CreateRoot();
        if (!root) return -1;

        if (!root->Init()) {
            LogError("Failed to initialize Root");
            return -1;
        }

        root->Run();
        delete root;
        return 0;
    }

} // namespace ballistic
