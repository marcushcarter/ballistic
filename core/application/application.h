#pragma once

struct Application {
    struct CreateInfo {

    };

    explicit Application(const CreateInfo& info) { (void)info; }
    virtual ~Application() {}

    int Run();

    CreateInfo create_info;
};