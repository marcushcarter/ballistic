#pragma once
#include <editor/editor_context.h>
#include <editor/editor_settings.h>
#include <editor/panel.h>
#include <core/log/error.h>
#include <memory>
#include <vector>

namespace ballistic {

struct Editor
{
    EditorContext context;
    std::vector<std::unique_ptr<Panel>> panels;
    
    EditorSettings settings;

    Error create(const EditorContext& p_context);
    void destroy();
    
    void on_update(float p_dt);

    void begin_dockspace();
    void draw_panels();
    void draw_menu();

    void load_settings();
    void save_settings();
};

}