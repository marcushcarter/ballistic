file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS "src/*.h")

add_library(ballistic_engine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

target_include_directories(ballistic_engine
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(ballistic_engine
    PUBLIC ballistic_compiler_options
    PUBLIC Vulkan::Vulkan
    PUBLIC vma
    PUBLIC glfw
    PUBLIC stb
    PUBLIC imgui
    PUBLIC glm
)

target_precompile_headers(ballistic_engine PRIVATE src/pch.h)