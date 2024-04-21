
function(__prepare_link_libs libs out)
    list(TRANSFORM libs STRIP)
    set("${out}" ${libs} PARENT_SCOPE)
endfunction()

function(make_test_project)

    set(__single_var_options TEST_NAME TEST_FOLDER SOURCE)
    set(__multi_var_options LIBRARIES)
    cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

    if (NOT DEFINED args_TEST_NAME)
        message(FATAL_ERROR "Target `${args_TEST_NAME}` is required parameter name to generate setup.")
        return()
    endif ()

    message(STATUS "Setup library project for target `${args_TEST_NAME}`.")
    name_to_target(target "${args_TEST_NAME}")

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
        set(source_directory "${proj_src_dir}/${args_TEST_NAME}")
    endif ()

    file(GLOB_RECURSE header_files CONFIGURE_DEPENDS ${source_directory}/*.hpp ${source_directory}/*.h ${source_directory}/*.hh)
    file(GLOB_RECURSE source_files CONFIGURE_DEPENDS ${source_directory}/*.cpp ${source_directory}/*.c ${source_directory}/*.cc)

    # Sources
    map_source_groups(BASE_PATH "${source_directory}" SOURCES ${source_files} ${header_files})

    # Executable
    add_executable(${target} EXCLUDE_FROM_ALL ${source_files} ${header_files})
    add_executable(${PROJECT_NAME}::${args_TEST_NAME} ALIAS ${target})

    # Project Options
    set_target_properties(${target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS})
    if (NOT DEFINED args_TEST_FOLDER)
        set_target_properties(${target} PROPERTIES FOLDER "${IDE_FOLDER}/Tests")
    elseif (DEFINED args_TEST_FOLDER)
        set_target_properties(${target} PROPERTIES FOLDER "${args_TEST_FOLDER}")
    endif ()

    # Include Directories
    target_include_directories(
            ${target}
            PRIVATE
            ${DEFAULT_INCLUDE_DIRECTORIES}
            ${TEST_INCLUDE_DIRECTORIES}
            ${proj_src_dir}
    )

    # Libraries
    target_link_libraries(
            ${target}
            PRIVATE
            ${DEFAULT_LIBRARIES}
            ${TEST_LIBRARIES}
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

    # Register Tests
    include(GoogleTest)
    gtest_discover_tests(${target} DISCOVERY_MODE PRE_TEST)

endfunction()