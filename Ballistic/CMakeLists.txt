file(GLOB_RECURSE ENGINE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp
)

add_library(Ballistic STATIC ${ENGINE_SRC})

target_include_directories(Ballistic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Ballistic PUBLIC
    glfw
    glad
    # glm
    imgui
    stb
    # EnTT::EnTT
    # tinyfiledialogs
    # nlohmann_json
    ThirdpartyImpl
    ${CMAKE_DL_LIBS}
)

target_precompile_headers(Ballistic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/bepch.h)