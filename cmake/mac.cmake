macro(mac_copy_bundle_files __TARGET __LOCATION __RESOURCES)

    foreach(__RESOURCE ${__RESOURCES})
        get_filename_component(__NAME ${__RESOURCE} NAME)

        if(IS_DIRECTORY "${__RESOURCE}")
            add_custom_command(TARGET ${__TARGET} POST_BUILD
                    COMMAND rm -rf "$<TARGET_FILE_DIR:${__TARGET}>/../${__LOCATION}/${__NAME}"
                    COMMAND cp -a "${__RESOURCE}" "$<TARGET_FILE_DIR:${__TARGET}>/../${__LOCATION}/${__NAME}"
                    VERBATIM)
        else()
            add_custom_command(TARGET ${__TARGET} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy "${__RESOURCE}" "$<TARGET_FILE_DIR:${__TARGET}>/../${__LOCATION}/${__NAME}"
                    COMMENT "Copying {__RESOURCE} to bundle")
        endif()
    endforeach()
endmacro()
