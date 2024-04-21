
function(__prepare_link_libs libs out)
    list(TRANSFORM libs STRIP)
    set("${out}" ${libs} PARENT_SCOPE)
endfunction()

function(make_library_project)

    set(__single_var_options LIB_NAME LIB_FOLDER SOURCE)
    set(__multi_var_options LIBRARIES)
    cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

    if (NOT DEFINED args_LIB_NAME)
        message(FATAL_ERROR "Target `${args_LIB_NAME}` is required parameter name to generate setup.")
        return()
    endif ()

    message(STATUS "Setup library project for target `${args_LIB_NAME}`.")
    name_to_target(target "${args_LIB_NAME}")

    list(LENGTH args_LIBRARIES link_libraries_size)
    if (link_libraries_size GREATER 0)
        __prepare_link_libs(${args_LIBRARIES} args_LIBRARIES)
        list(LENGTH args_LIBRARIES link_libraries_size)
    endif ()
    message(STATUS "Using `${link_libraries_size}` libraries at linker.")

    # Discover Sources
    get_src_path(proj_src_dir)

    if (DEFINED args_SOURCE)
        set(source_directory "${proj_src_dir}/${args_SOURCE}")
    else ()
        set(source_directory "${proj_src_dir}/${args_LIB_NAME}")
    endif ()

    file(GLOB_RECURSE header_files ${source_directory}/*.hpp ${source_directory}/*.h ${source_directory}/*.hh)
    file(GLOB_RECURSE source_files ${source_directory}/*.cpp ${source_directory}/*.c ${source_directory}/*.cc)

    # Sources
    map_source_groups(BASE_PATH "${source_directory}" SOURCES ${source_files} ${header_files})

    # Library
    add_library(${target} ${source_files} ${header_files})
    add_library(${PROJECT_NAME}::${args_LIB_NAME} ALIAS ${target})

    # Export Downstream
    export(TARGETS ${target} NAMESPACE ${PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

    # Project Options
    set_target_properties(${target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS})
    if (NOT DEFINED args_LIB_FOLDER)
        try_generate_project_groups("${args_LIB_NAME}" may_sub_folder)
        if (DEFINED may_sub_folder)
            set_target_properties(${target} PROPERTIES FOLDER "${IDE_FOLDER}/${may_sub_folder}")
        endif ()

    elseif (DEFINED args_LIB_FOLDER)
        set_target_properties(${target} PROPERTIES FOLDER "${args_LIB_FOLDER}")
    endif ()

    # Include Directories
    target_include_directories(
            ${target}
            PRIVATE
            ${DEFAULT_INCLUDE_DIRECTORIES}
            ${proj_src_dir}
    )

    # Libraries
    target_link_libraries(
            ${target}
            PRIVATE
            ${DEFAULT_LIBRARIES}
            ${args_LIBRARIES}
    )

    # Compile
    target_compile_definitions(
            ${target}
            PRIVATE
            ${DEFAULT_COMPILE_DEFINITIONS}
    )

    # Compile Options
    target_compile_options(
            ${target}
            PRIVATE
            ${DEFAULT_COMPILE_OPTIONS}
    )

    # Linker Options
    target_link_options(
            ${target}
            PRIVATE
            ${DEFAULT_LINKER_OPTIONS}
    )

    # Target Health
    # Deployment
    get_shared_dist_path(proj_dist_path)
    set_batch_target_properties(${target} ${proj_dist_path})
endfunction()