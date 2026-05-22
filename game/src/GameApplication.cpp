#include "GameApplication.h"
#include <cstdio>

void GameApplication::OnInit()
{
    printf("Game initialized\n");
    window.SetFullscreen(true);
}

void GameApplication::OnUpdate()
{
    printf("Game update\n");
}

void GameApplication::OnShutdown()
{
    printf("Game shutdown\n");
}
