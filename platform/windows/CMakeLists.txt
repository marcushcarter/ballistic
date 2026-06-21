function(ballistic_common_target target)
    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/generated
    )
    target_link_options(${target} PRIVATE /MANIFEST:NO)
endfunction()

if(BALLISTIC_BUILD_EDITOR)
    add_executable(ballistic_editor_app WIN32
        resources/editor.rc
        ${CMAKE_BINARY_DIR}/generated/embedded_assets_shared.rc
        ${CMAKE_BINARY_DIR}/generated/embedded_assets_editor.rc
        main_editor.cpp
    )
    target_link_libraries(ballistic_editor_app PRIVATE ballistic_core)
    ballistic_common_target(ballistic_editor_app)
    target_compile_definitions(ballistic_editor_app PRIVATE BALLISTIC_EDITOR)
endif()

if(BALLISTIC_BUILD_GAME)
    add_executable(ballistic_game WIN32
        resources/game.rc
        ${CMAKE_BINARY_DIR}/generated/embedded_assets_shared.rc
        main_game.cpp
    )
    target_link_libraries(ballistic_game PRIVATE ballistic_core)
    ballistic_common_target(ballistic_game)
    target_compile_definitions(ballistic_game PRIVATE BALLISTIC_GAME)
endif()

if(BALLISTIC_BUILD_CONSOLE_WRAPPER)
    add_executable(ballistic_console_wrapper
        resources/console_wrapper.rc
        console_wrapper.cpp
    )
    ballistic_common_target(ballistic_console_wrapper)
endif()