file(GLOB_RECURSE ENGINE_SRC
    src/*.cpp
    src/Core/*.cpp
    src/Platform/*.cpp
    src/Project/*.cpp
    src/Renderer/*.cpp
)

add_library(Ballistic STATIC ${ENGINE_SRC})

target_include_directories(Ballistic PUBLIC src)

target_link_libraries(Ballistic PUBLIC
    glfw
    glad
    glm
    imgui
    stb
    EnTT::EnTT
    tinyfiledialogs
    nlohmann_json
    ${CMAKE_DL_LIBS}
)

target_precompile_headers(Ballistic PUBLIC src/bepch.h)