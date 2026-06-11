file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")

add_library(ballistic_platform STATIC ${PLATFORM_SOURCES})

target_include_directories(ballistic_platform
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ballistic_platform
    PUBLIC
        glfw
        dwmapi
)