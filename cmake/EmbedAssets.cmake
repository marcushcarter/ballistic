function(ballistic_generate_asset_rc OUTPUT_RC)
    set(ASSET_DIRS ${ARGN})
    set(RC_CONTENT "")

    get_filename_component(OUTPUT_NAME_WE ${OUTPUT_RC} NAME_WE)
    string(TOUPPER "${OUTPUT_NAME_WE}" OUTPUT_PREFIX)

    foreach(DIR ${ASSET_DIRS})
        if(NOT EXISTS ${DIR})
            continue()
        endif()

        file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS "${DIR}/*")
        get_filename_component(DIR_NAME ${DIR} NAME)

        foreach(FILE ${ASSET_FILES})
            file(RELATIVE_PATH REL_PATH ${DIR} ${FILE})
            string(REGEX REPLACE "[^A-Za-z0-9]" "_" SAFE_NAME ${REL_PATH})
            string(TOUPPER "${OUTPUT_PREFIX}_${DIR_NAME}_${SAFE_NAME}" RES_NAME)
            string(APPEND RC_CONTENT "${RES_NAME} RCDATA \"${FILE}\"\n")
        endforeach()
    endforeach()

    file(WRITE ${OUTPUT_RC} "${RC_CONTENT}")
endfunction()