
file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE EDITOR_HEADERS CONFIGURE_DEPENDS "src/*.h")

add_executable(ballistic_editor_app WIN32
    ${EDITOR_SOURCES}
    ${EDITOR_HEADERS}
    res/editor_app.rc
    ${BALLISTIC_ENGINE_RC_FILES}
)

target_link_libraries(ballistic_editor_app PRIVATE ballistic_engine)
target_compile_definitions(ballistic_editor_app PRIVATE BALLISTIC_EDITOR)
target_precompile_headers(ballistic_editor_app REUSE_FROM ballistic_engine)
set_target_properties(ballistic_editor_app PROPERTIES OUTPUT_NAME "Ballistic_v${BALLISTIC_VERSION}_win64")

add_executable(ballistic_editor
    stub/console_stub.cpp
    res/editor_console.rc
)
target_compile_definitions(ballistic_editor PRIVATE BALLISTIC_VERSION_STR="${BALLISTIC_VERSION}")
set_target_properties(ballistic_editor PROPERTIES OUTPUT_NAME "Ballistic_v${BALLISTIC_VERSION}_win64_console")