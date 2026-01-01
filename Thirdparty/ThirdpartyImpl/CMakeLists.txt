add_library(ThirdpartyImpl STATIC
    stb_image_impl.cpp
)

target_include_directories(ThirdpartyImpl PUBLIC ${CMAKE_SOURCE_DIR}/Thirdparty/stb)
