file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS "src/*.h")

add_executable(ballistic_game
    ${GAME_SOURCES}
    ${GAME_HEADERS}
)

target_link_libraries(ballistic_game
    PRIVATE ballistic_engine
)