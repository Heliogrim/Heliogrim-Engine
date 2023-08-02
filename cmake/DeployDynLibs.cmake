
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

    get_library_path(proj_lib_dir)
    file(GLOB_RECURSE deployable ${proj_lib_dir}/bin/**/${CMAKE_BUILD_TYPE}/*.dll)
    list(LENGTH deployable deployable_size)

    if (deployable_size LESS_EQUAL 0)
        message(STATUS "Exit dynamic lib deployment, cause there are none.")
        return()
    endif ()

    get_dist_path(proj_dist_dir)
    add_custom_command(
            TARGET ${target}
            POST_BUILD COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${deployable} "${proj_dist_dir}/${CMAKE_BUILD_TYPE}"
    )
endfunction()