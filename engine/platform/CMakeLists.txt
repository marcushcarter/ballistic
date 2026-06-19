file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")

add_library(ballistic_platform STATIC ${PLATFORM_SOURCES})

target_include_directories(ballistic_platform
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ballistic_platform
PUBLIC
    glfw
    imgui
PRIVATE
    dwmapi
)

target_compile_definitions(ballistic_platform PRIVATE BALLISTIC_VERSION_STR="${BALLISTIC_VERSION}")