file(GLOB_RECURSE ENGINE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Entry/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Root/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Utility/*.cpp
)

add_library(Ballistic STATIC ${ENGINE_SRC})

target_include_directories(Ballistic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Ballistic PUBLIC
    glfw
    glad
    glm
    imgui
    ImGuizmo
    stb
    EnTT::EnTT
    assimp
    tinyfiledialogs
    nlohmann_json
    ThirdpartyImpl
    ${CMAKE_DL_LIBS}
)

target_precompile_headers(Ballistic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/bepch.h)