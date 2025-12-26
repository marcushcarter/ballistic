file(GLOB_RECURSE ENGINE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Project/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/*.cpp
)

add_library(Engine STATIC ${ENGINE_SRC})

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Engine PUBLIC
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

target_precompile_headers(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/bepch.h)