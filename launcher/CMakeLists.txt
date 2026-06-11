file(GLOB_RECURSE LAUNCHER_SOURCES
    CONFIGURE_DEPENDS
    "*.cpp"
    "*.h"
)

add_executable(ballistic_launcher
    ${LAUNCHER_SOURCES}
)

target_include_directories(ballistic_launcher
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(ballistic_launcher
PRIVATE
    ballistic_platform
    ballistic_vulkan
    # ballistic_project
    imgui
    IconFontCppHeaders
)

set_target_properties(ballistic_launcher PROPERTIES
    FOLDER "Tools"
)