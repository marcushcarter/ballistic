#pragma once
#include <editor/editor_context.h>
#include <editor/panel.h>
#include <core/log/error.h>
#include <memory>
#include <vector>

namespace ballistic {

struct Editor
{
    EditorContext context;
    
    std::vector<std::unique_ptr<Panel>> panels;
    
    bool close_project_requested = false;

    Error create(const EditorContext& p_context);
    void destroy();
    
    void on_update(float p_dt);

    void begin_dockspace();
    void draw_panels();
    void draw_menu();

    void apply_settings();
    void store_settings();
};

}