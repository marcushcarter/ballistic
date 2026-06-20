
#pragma once
#include <core/application/application.h>

namespace ballistic {

struct EditorApplication : Application
{
    void on_init() override;
    void on_update(float p_dt) override;
    void on_shutdown() override;
};

}