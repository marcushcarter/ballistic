#include <core/application/editor_application.h>
#include <core/io/embedded_resource.h>

namespace ballistic {

void EditorApplication::on_init()
{
    window.set_icon(EmbeddedResource::load_icon(L"BALLISTIC_ICON"));
    window.set_titlebar_color(RGB(20, 20, 25));
}

void EditorApplication::on_update(float p_dt)
{
    (void)p_dt;
}

void EditorApplication::on_shutdown()
{
    
}

}