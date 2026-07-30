#pragma once
#include <editor/editor_context.h>
#include <editor/panel/panel.h>
#include <core/base/error.h>
#include <memory>
#include <vector>
#include <map>

namespace ballistic {

struct Editor
{    
    std::vector<std::unique_ptr<Panel>> panels;
    std::map<std::string, bool> panel_open;

    Error initialize();
    void shutdown();
    
    void on_update(EditorContext& ctx, float p_dt);
    
    void begin_dockspace();
    void draw_menu();

    void apply_settings();
    void store_settings();
};

}