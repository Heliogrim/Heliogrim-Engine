
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

    get_target_property(link_libs ${target} LINK_LIBRARIES)
    set(dlls "")
    foreach (candidate ${link_libs})
        if (TARGET ${candidate})
            get_target_property(candidate_type ${candidate} TYPE)
            if (candidate_type STREQUAL "SHARED_LIBRARY")
                get_target_property(candidate_run_dll_path ${candidate} RUNTIME_OUTPUT_DIRECTORY)
                get_target_property(candidate_name ${candidate} NAME)

                if (DEFINED candidate_run_dll_path AND DEFINED candidate_name)
                    list(APPEND dlls "${candidate_run_dll_path}**/${candidate_name}**.dll")
                endif ()
            endif ()
        endif ()
    endforeach ()

    get_library_path(proj_lib_dir)
    gen_target_name_id(name_id)
    file(GLOB_RECURSE deployable "${proj_lib_dir}/bin/**/$<CONFIG>**/${name_id}**/*.dll")
    list(LENGTH deployable deployable_size)

    if (deployable_size LESS_EQUAL 0)
        message(STATUS "Exit dynamic lib deployment, cause there are none.")
        return()
    endif ()

    get_shared_dist_path(proj_dist_dir)
    add_custom_command(
            TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${deployable} "${proj_dist_dir}"
            #COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dlls} "${proj_dist_dir}"
    )
endfunction()