#pragma once
#include <core/dev_tools/dev_context.h>
#include <core/dev_tools/dev_panel.h>
#include <core/log/error.h>
#include <memory>
#include <vector>

namespace ballistic {

struct DevTools
{    
    DevContext context;
    std::vector<std::unique_ptr<DevPanel>> panels;

    Error create(const DevContext& p_context);
    void destroy();
    
    void draw_panels(bool p_editor = false);
    void draw_menu(bool p_editor = false);
};

}