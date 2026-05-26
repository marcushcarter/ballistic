file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS "src/*.h")

add_executable(ballistic_game # WIN32
    ${GAME_SOURCES}
    ${GAME_HEADERS}
    ${BALLISTIC_ENGINE_RC_FILES}
)

target_link_libraries(ballistic_game PRIVATE ballistic_engine)

target_precompile_headers(ballistic_game REUSE_FROM ballistic_engine)

set_target_properties(ballistic_game PROPERTIES OUTPUT_NAME "ballistic_game")