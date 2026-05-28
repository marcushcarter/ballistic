file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS "src/*.h")

add_subdirectory(res)

add_library(ballistic_engine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

add_dependencies(ballistic_engine CompileEngineShaders)

target_include_directories(ballistic_engine
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PUBLIC "${CMAKE_CURRENT_BINARY_DIR}"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/res"
)

target_link_libraries(ballistic_engine
    PUBLIC ballistic_compiler_options
    PUBLIC Vulkan::Vulkan
    PUBLIC vma
    PUBLIC glfw
    PUBLIC stb
    PUBLIC imgui
    PUBLIC glm
    PUBLIC iconfontcppheaders
    PUBLIC tinyfiledialogs
    PUBLIC tomlplusplus
    PUBLIC Shell32
    PUBLIC imnodes
)

target_precompile_headers(ballistic_engine PRIVATE src/pch.h)