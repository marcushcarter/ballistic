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

    Error create(const EditorContext& p_context);
    void destroy();
    void update(float p_dt);
    void draw();

    void begin_dockspace();
    void draw_menu_bar();

    void load_settings();
    void save_settings();
};

}