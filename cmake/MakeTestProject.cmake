
function(__prepare_link_libs libs out)
    list(TRANSFORM libs STRIP)
    set("${out}" ${libs} PARENT_SCOPE)
endfunction()

function(make_test_project target link_libraries)
    if (NOT DEFINED target)
        message(FATAL_ERROR "Target `${target}` is required parameter name to generate setup.")
        return()
    endif ()

    message(STATUS "Setup library project for target `${target}`.")

    __prepare_link_libs(${link_libraries} link_libraries)
    list(LENGTH link_libraries link_libraries_size)
    message(STATUS "Using `${link_libraries_size}` libraries at linker.")

    # Discover Sources
    set(source_directory "${META_PROJECT_SRC_DIR}/${target}")

    file(GLOB_RECURSE header_files ${source_directory}/*.hpp ${source_directory}/*.h ${source_directory}/*.hh)
    file(GLOB_RECURSE source_files ${source_directory}/*.cpp ${source_directory}/*.c ${source_directory}/*.cc)

    # Sources
    set(headers ${header_files})
    set(sources ${source_files})

    # Executable
    add_executable(${target} ${sources} ${headers})
    add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

    # Project Options
    set_target_properties(${target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS} FOLDER "${IDE_FOLDER}")

    # Include Directories
    target_include_directories(
            ${target}
            PRIVATE
            ${DEFAULT_INCLUDE_DIRECTORIES}
            ${TEST_INCLUDE_DIRECTORIES}
            ${META_PROJECT_SRC_DIR}
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

    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${META_PROJECT_DIST_DIR}/$<CONFIG>)
    set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${META_PROJECT_DIST_DIR}/$<CONFIG>)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${META_PROJECT_DIST_DIR}/$<CONFIG>)
endfunction()