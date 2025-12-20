file(GLOB_RECURSE EDITOR_SRC src/*.cpp)

if(EDITOR_SRC)
    add_executable(BallisticEditor ${EDITOR_SRC})

    # Windows icon
    if(WIN32)
        set(RUNTIME_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")
        target_sources(BallisticEditor PRIVATE ${RUNTIME_ICON})
    endif()

    # Include the src folder so Panels/IPanel.h can be found
    target_include_directories(BallisticEditor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(BallisticEditor PRIVATE Ballistic)
endif()
