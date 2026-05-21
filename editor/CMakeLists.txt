file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE EDITOR_HEADERS CONFIGURE_DEPENDS "src/*.h")

add_executable(ballistic_editor
    ${EDITOR_SOURCES}
    ${EDITOR_HEADERS}
    resources/editor_console.rc
)

add_executable(ballistic_editor_app WIN32
    ${EDITOR_SOURCES}
    ${EDITOR_HEADERS}
    resources/editor_app.rc
)

foreach(TARGET ballistic_editor ballistic_editor_app)
    target_link_libraries(${TARGET} PRIVATE ballistic_engine)
    target_compile_definitions(${TARGET} PRIVATE BALLISTIC_EDITOR)
    target_precompile_headers(${TARGET} REUSE_FROM ballistic_engine)
endforeach()

target_compile_definitions(ballistic_editor PRIVATE BALLISTIC_CONSOLE)

set_target_properties(ballistic_editor PROPERTIES OUTPUT_NAME "Ballistic_v${BALLISTIC_VERSION}_win64_console")

set_target_properties(ballistic_editor_app PROPERTIES OUTPUT_NAME "Ballistic_v${BALLISTIC_VERSION}_win64")