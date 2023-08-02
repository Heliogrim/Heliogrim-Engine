
function(__prepare_link_libs libs out)
    list(TRANSFORM libs STRIP)
    set("${out}" ${libs} PARENT_SCOPE)
endfunction()

function(make_test_project name link_libraries)
    if (NOT DEFINED name)
        message(FATAL_ERROR "Target `${name}` is required parameter name to generate setup.")
        return()
    endif ()

    message(STATUS "Setup library project for target `${name}`.")
    name_to_target(target "${name}")

    list(LENGTH link_libraries link_libraries_size)
    if (link_libraries_size GREATER 0)
        __prepare_link_libs(${link_libraries} link_libraries)
        list(LENGTH link_libraries link_libraries_size)
    endif ()
    message(STATUS "Using `${link_libraries_size}` libraries at linker.")

    # Discover Sources
    get_src_path(proj_src_dir)
    set(source_directory "${proj_src_dir}/${name}")

    file(GLOB_RECURSE header_files ${source_directory}/*.hpp ${source_directory}/*.h ${source_directory}/*.hh)
    file(GLOB_RECURSE source_files ${source_directory}/*.cpp ${source_directory}/*.c ${source_directory}/*.cc)

    # Sources
    set(headers ${header_files})
    set(sources ${source_files})

    # Executable
    add_executable(${target} EXCLUDE_FROM_ALL ${sources} ${headers})
    add_executable(${PROJECT_NAME}::${name} ALIAS ${target})

    # Project Options
    set_target_properties(${target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS} FOLDER "${IDE_FOLDER}")

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
            ${link_libraries}
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
    get_dist_path(proj_dist_path)
    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${proj_dist_path}/$<CONFIG>)
    set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${proj_dist_path}/$<CONFIG>)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${proj_dist_path}/$<CONFIG>)
endfunction()