function(ballistic_common_target target)
    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/generated
    )
    target_link_options(${target} PRIVATE /MANIFEST:NO)
endfunction()

set(BALLISTIC_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/ballistic_main.cpp)
set(BALLISTIC_WRAPPER ${CMAKE_CURRENT_SOURCE_DIR}/ballistic_console_wrapper.cpp)

if(BALLISTIC_BUILD_EDITOR)
    add_executable(ballistic_editor_app WIN32
        ballistic_res.rc
        ${CMAKE_BINARY_DIR}/generated/embedded_assets_shared.rc
        ${CMAKE_BINARY_DIR}/generated/embedded_assets_editor.rc
        ${BALLISTIC_MAIN}
    )
    target_link_libraries(ballistic_editor_app PRIVATE ballistic_core ballistic_editor)
    target_compile_definitions(ballistic_editor_app PRIVATE BALLISTIC_EDITOR)
    ballistic_common_target(ballistic_editor_app)

    add_executable(ballistic_console_wrapper
        ballistic_res_console.rc
        ${BALLISTIC_WRAPPER}
    )
    target_compile_definitions(ballistic_console_wrapper PRIVATE "BALLISTIC_WRAPPER_TARGET=L\"ballistic_editor_app.exe\"")
    ballistic_common_target(ballistic_console_wrapper)
endif()

if(BALLISTIC_BUILD_GAME)
    add_executable(ballistic_game WIN32
        ballistic_res_app.rc
        ${CMAKE_BINARY_DIR}/generated/embedded_assets_shared.rc
        ${BALLISTIC_MAIN}
        ${CMAKE_SOURCE_DIR}/game/game_application.cpp
    )
    target_link_libraries(ballistic_game PRIVATE ballistic_core)
    target_compile_definitions(ballistic_game PRIVATE BALLISTIC_GAME)
    ballistic_common_target(ballistic_game)

    add_executable(ballistic_game_dev_tools
        ballistic_res_app_dev.rc
        ${CMAKE_BINARY_DIR}/generated/embedded_assets_shared.rc
        ${BALLISTIC_MAIN}
        ${CMAKE_SOURCE_DIR}/game/game_application.cpp
    )
    target_link_libraries(ballistic_game_dev_tools PRIVATE ballistic_core)
    target_compile_definitions(ballistic_game_dev_tools PRIVATE BALLISTIC_GAME BALLISTIC_DEV_TOOLS BALLISTIC_CONSOLE)
    ballistic_common_target(ballistic_game_dev_tools)
endif()