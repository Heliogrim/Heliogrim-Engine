
function(__prepare_link_libs libs out)
    list(TRANSFORM libs STRIP)
    set("${out}" ${libs} PARENT_SCOPE)
endfunction()

function(make_app_project)

    set(__single_var_options APP_NAME APP_FOLDER LIB_NAME LIB_FOLDER SOURCE)
    set(__multi_var_options LIBRARIES)
    cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

    if (NOT DEFINED args_APP_NAME)
        message(FATAL_ERROR "Target `${args_APP_NAME}` is required parameter name to generate setup.")
        return()
    endif ()

    if (NOT DEFINED args_LIB_NAME)
        message(STATUS "Using `${args_APP_NAME}` as root to autogenerate library name.")
        set(args_LIB_NAME "${args_APP_NAME}-Lib")
    endif ()

    message(STATUS "Setup executable and library project for target `${args_APP_NAME}` and `${args_LIB_NAME}` respectively.")
    name_to_target(app_target ${args_APP_NAME})
    name_to_target(lib_target ${args_LIB_NAME})

    # Prepare Link Libraries
    list(LENGTH args_LIBRARIES link_libraries_size)
    if (link_libraries_size GREATER 0)
        __prepare_link_libs(${args_LIBRARIES} args_LIBRARIES)
        list(LENGTH args_LIBRARIES link_libraries_size)
    endif ()
    message(STATUS "Using `${link_libraries_size}` libraries at linker.")

    # Discover Sources
    get_src_path(proj_src_dir)

    if (NOT DEFINED args_SOURCE)
        if (EXISTS "${proj_src_dir}/${args_APP_NAME}")
            set(source_directory "${proj_src_dir}/${args_APP_NAME}")
        elseif (EXISTS "${proj_src_dir}/${args_LIB_NAME}")
            set(source_directory "${proj_src_dir}/${args_LIB_NAME}")
        else ()
            set(source_directory "${proj_src_dir}/${args_APP_NAME}")
        endif ()
    else ()
        set(source_directory "${proj_src_dir}/${args_SOURCE}")
    endif ()

    file(GLOB_RECURSE header_files CONFIGURE_DEPENDS ${source_directory}/*.hpp ${source_directory}/*.h ${source_directory}/*.hh)
    file(GLOB_RECURSE source_files CONFIGURE_DEPENDS ${source_directory}/*.cpp ${source_directory}/*.c ${source_directory}/*.cc)

    # Sources
    map_source_groups(BASE_PATH "${source_directory}" SOURCES ${source_files} ${header_files})

    # Executable
    add_executable(${app_target} ${source_files} ${header_files})
    add_executable(${PROJECT_NAME}::${args_APP_NAME} ALIAS ${app_target})

    # Library
    add_library(${lib_target} ${source_files} ${header_files})
    add_library(${PROJECT_NAME}::${args_LIB_NAME} ALIAS ${lib_target})

    # Export Downstream
    export(TARGETS ${lib_target} NAMESPACE ${PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${lib_target}/${lib_target}-export.cmake)

    # Project Options
    set_target_properties(${app_target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS} FOLDER "${IDE_FOLDER}")
    set_target_properties(${lib_target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS} FOLDER "${IDE_FOLDER}")

    if (NOT DEFINED args_APP_FOLDER)
        try_generate_project_groups("${args_APP_NAME}" may_sub_folder)
        if (DEFINED may_sub_folder)
            set_target_properties(${app_target} PROPERTIES FOLDER "${IDE_FOLDER}/${may_sub_folder}")
        endif ()

    elseif (DEFINED args_APP_FOLDER)
        set_target_properties(${app_target} PROPERTIES FOLDER "${args_APP_FOLDER}")
    endif ()

    if (NOT DEFINED args_LIB_FOLDER)
        try_generate_project_groups("${args_LIB_NAME}" may_sub_folder)
        if (DEFINED may_sub_folder)
            set_target_properties(${lib_target} PROPERTIES FOLDER "${IDE_FOLDER}/${may_sub_folder}")
        endif ()

    elseif (DEFINED args_LIB_FOLDER)
        set_target_properties(${lib_target} PROPERTIES FOLDER "${args_LIB_FOLDER}")
    endif ()

    # Include Directories
    target_include_directories(${app_target} PRIVATE ${proj_src_dir})
    target_include_directories(${app_target} SYSTEM PUBLIC ${DEFAULT_INCLUDE_DIRECTORIES})

    target_include_directories(${lib_target} PRIVATE ${proj_src_dir})
    target_include_directories(${lib_target} SYSTEM PUBLIC ${DEFAULT_INCLUDE_DIRECTORIES})

    # Link Libraries
    target_link_libraries(
            ${app_target}
            PRIVATE
            ${args_LIBRARIES}
            ${DEFAULT_LIBRARIES}
    )
    target_link_libraries(
            ${lib_target}
            PRIVATE
            ${args_LIBRARIES}
            ${DEFAULT_LIBRARIES}
    )

    # Compile Definitions
    target_compile_definitions(
            ${app_target}
            PRIVATE
            ${DEFAULT_COMPILE_DEFINITIONS}
    )

    # Compile Options
    target_compile_options(
            ${app_target}
            PRIVATE
            ${DEFAULT_COMPILE_OPTIONS}
    )
    target_compile_options(
            ${lib_target}
            PRIVATE
            ${DEFAULT_COMPILE_OPTIONS}
    )

    # Linker Options
    target_link_options(
            ${app_target}
            PRIVATE
            ${DEFAULT_LINKER_OPTIONS}
    )
    target_link_options(
            ${lib_target}
            PRIVATE
            ${DEFAULT_LINKER_OPTIONS}
    )

    # Target Health
    get_shared_dist_path(proj_dist_dir)
    set_batch_target_properties(${app_target} ${proj_dist_dir})
    set_batch_target_properties(${lib_target} ${proj_dist_dir})

    # Deployment
    add_deploy_to_target(${app_target})

endfunction()