
set(OPTION_DEPLOY_ENABLED ON)

function(enable_deploy status)
    if (NOT ${status})
        set(OPTION_DEPLOY_ENABLED ${status} PARENT_SCOPE)
        message(STATUS "Deployment of dlls skipped: Manually disabled")

        return()
    endif ()
endfunction()

function(add_deploy_to_target target)

    if (NOT ${OPTION_DEPLOY_ENABLED})
        message(STATUS "Skipped deployment: Manually disabled")
        return()
    endif ()

    file(GLOB_RECURSE deployable ${META_PROJECT_LIB_DIR}/bin/**/${CMAKE_BUILD_TYPE}/*.dll)

    add_custom_command(
            TARGET ${target}
            POST_BUILD COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${deployable} "${META_PROJECT_DIST_DIR}/${CMAKE_BUILD_TYPE}"
    )
endfunction()