function(library_gli_init)

    cmake_minimum_required(VERSION 3.27.7)
    include(FetchContent)

    name_to_external_alias(alias "gli")

    # Include Sub-Project
    get_external_include_path(lib_inc_path "gli")
    get_external_source_path(lib_src_path "gli")

    CachedFetchContent_Declare(
            gli
            GIT_REPOSITORY https://github.com/g-truc/gli.git
            GIT_TAG master
            GIT_SUBMODULES_RECURSE FALSE
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            SOURCE_DIR "${lib_src_path}"
            PATCH_COMMAND ${CMAKE_COMMAND} -E rm -f -- "${lib_src_path}/CMakeLists.txt"
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(
            gli
    )

    # Setup Library
    install_includes(
            DIRECTORY "${lib_src_path}/gli/"
            DESTINATION "${lib_inc_path}/gli/"
    )

endfunction()

# Push to setup
add_managed_library(NAME gli CALL library_gli_init ORDER_INDEPENDENT ON)