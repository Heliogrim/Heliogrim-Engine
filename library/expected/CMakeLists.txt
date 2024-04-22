function(library_expected_init)

    cmake_minimum_required(VERSION 3.27.7)
    include(FetchContent)

    name_to_external_alias(alias "expected")

    # Include Sub-Project
    get_external_include_path(lib_inc_path "expected")
    get_external_source_path(lib_src_path "expected")

    CachedFetchContent_Declare(
            expected
            GIT_REPOSITORY https://github.com/TartanLlama/expected.git
            GIT_TAG v1.1.0
            GIT_SUBMODULES_RECURSE FALSE
            GIT_SHALLOW TRUE
            SOURCE_DIR "${lib_src_path}"
            PATCH_COMMAND ${CMAKE_COMMAND} -E rm -f -- "${lib_src_path}/CMakeLists.txt"
            EXCLUDE_FROM_ALL
            OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(
            expected
    )

    # Setup Library
    install_includes(
            DIRECTORY "${lib_src_path}/include/"
            DESTINATION "${lib_inc_path}/"
    )

endfunction()

# Push to setup
add_managed_library(NAME expected CALL library_expected_init ORDER_INDEPENDENT ON)