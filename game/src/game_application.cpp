#include "game_application.h"
#include <cstdio>

void GameApplication::OnInit()
{
    printf("Game initialized\n");
}

void GameApplication::OnUpdate()
{
    printf("Game update\n");
}

void GameApplication::OnShutdown()
{
    printf("Game shutdown\n");
}
