#include <core/application/game_application.h>

namespace ballistic {

Error GameApplication::on_init()
{
    return Error::Ok;
}

void GameApplication::on_update(float p_dt)
{
    (void)p_dt;
}

void GameApplication::on_shutdown()
{
    
}
    
}