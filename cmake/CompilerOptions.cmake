
add_library(ballistic_compiler_options INTERFACE)

target_compile_options(ballistic_compiler_options INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>: /W4 /WX /permissive- /Zc:__cplusplus /MP>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>: -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter>
)