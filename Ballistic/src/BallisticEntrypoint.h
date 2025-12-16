#pragma once

#include "Core/Application.h"

extern Ballistic::Application* Ballistic::CreateApplication(const std::filesystem::path& exeDir);

int main(int argc, char** argv) {
    std::filesystem::path exePath = std::filesystem::weakly_canonical(argv[0]);
    std::filesystem::path exeDir = exePath.parent_path();
    
    Ballistic::Application* app = Ballistic::CreateApplication(exeDir);
    app->run();
    delete app;
}