#include "EditorApplication.h"
#include <cstdio>

void EditorApplication::OnInit()
{
    LOG_DEBUG("Editor initialized");
    window.DefaultIcon();
    window.SetTitlebarColor(0.2f, 0.2f, 0.2f);
}

void EditorApplication::OnUpdate()
{
    // LOG_TRACE("Editor update");
}

void EditorApplication::OnShutdown()
{
    LOG_DEBUG("Editor shutdown");
}
