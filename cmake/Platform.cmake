if(WIN32)
    target_compile_definitions(ballistic_compiler_options INTERFACE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
        VK_USE_PLATFORM_WIN32_KHR
    )
endif()