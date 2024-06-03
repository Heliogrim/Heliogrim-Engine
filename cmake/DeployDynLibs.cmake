
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
    gen_target_name_id(name_id)

    set(deployable "")
    foreach (candidate ${link_libs})
        if (TARGET ${candidate})
            get_target_property(candidate_type ${candidate} TYPE)
            if (candidate_type STREQUAL "SHARED_LIBRARY")
                get_target_property(candidate_run_dll_path ${candidate} RUNTIME_OUTPUT_DIRECTORY)
                get_target_property(candidate_run_dll_name ${candidate} RUNTIME_OUTPUT_NAME)
                if (NOT ${candidate_run_dll_name})
                    get_target_property(candidate_run_dll_name ${candidate} OUTPUT_NAME)
                endif ()

                if (DEFINED candidate_run_dll_path AND DEFINED candidate_run_dll_name)
                    # file(GLOB_RECURSE deployable "${proj_lib_dir}/**/bin/**/$<CONFIG>**/${name_id}**/*.dll")
                    list(APPEND deployable "${candidate_run_dll_path}/${candidate_run_dll_name}$<$<CONFIG:Debug>:${CMAKE_DEBUG_POSTFIX}>.dll")
                endif ()
            endif ()
        endif ()
    endforeach ()

    # Append Sanitizer DLLs
    if (SANITIZER_DEPLOY_DLLS)
        foreach (pattern IN LISTS SANITIZER_DEPLOY_DLLS)
            file(GLOB tmp "${pattern}")
            list(APPEND deployable ${tmp})
        endforeach ()
    endif ()


    list(REMOVE_DUPLICATES deployable)
    list(LENGTH deployable deployable_size)

    if (deployable_size LESS_EQUAL 0)
        message(STATUS "Exit dynamic lib deployment, cause there are none.")
        return()
    endif ()

    get_shared_dist_path(proj_dist_dir)
    add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${deployable} "${proj_dist_dir}"
    )
endfunction()