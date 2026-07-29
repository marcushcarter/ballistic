#pragma once
#include <editor/editor_context.h>
#include <editor/panel/panel.h>
#include <core/log/error.h>
#include <memory>
#include <vector>
#include <map>

namespace ballistic {

struct Editor
{
    EditorContext context;
    
    std::vector<std::unique_ptr<Panel>> panels;
    std::map<std::string, bool> panel_open;
    
    bool close_project_requested = false;

    int active_tab = 0;

    Error create(const EditorContext& p_context);
    void destroy();
    
    void on_update(float p_dt);
    
    void begin_dockspace();
    void draw_menu();

    void apply_settings();
    void store_settings();
};

}