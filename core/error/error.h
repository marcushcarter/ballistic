#pragma once

namespace ballistic {

enum class Error 
{
    Ok,
    Failed,
    Max
};

extern const char *error_names[];

}