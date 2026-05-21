#include "editor_application.h"
#include <cstdio>

void EditorApplication::OnInit()
{
    printf("Editor initialized\n");
    window.DefaultIcon();
}

void EditorApplication::OnUpdate()
{
    printf("Editor update\n");
}

void EditorApplication::OnShutdown()
{
    printf("Editor shutdown\n");
}
