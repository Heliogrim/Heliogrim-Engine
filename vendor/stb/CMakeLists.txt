function(library_stb_init)

    cmake_minimum_required(VERSION 3.25.1)
    include(FetchContent)

    name_to_external_alias(alias "stb")

    # Include Sub-Project
    get_external_include_path(vendor_inc_path "stb")
    get_external_source_path(vendor_src_path "stb")

    CachedFetchContent_Declare(
            stb
            GIT_REPOSITORY https://github.com/nothings/stb.git
            GIT_TAG master
            GIT_SUBMODULES_RECURSE FALSE
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            SOURCE_DIR "${vendor_src_path}"
            PATCH_COMMAND ${CMAKE_COMMAND} -E rm -f -- "${vendor_src_path}/CMakeLists.txt"
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(
            stb
    )

    # Setup Library
    file(GLOB headers LIST_DIRECTORIES false "${vendor_src_path}/*.*")
    list(FILTER headers INCLUDE REGEX "stb_.+\.(h|c)")
    install_includes(
            SOURCE "${headers}"
            DESTINATION "${vendor_inc_path}/stb/"
    )

endfunction()

# Push to setup
add_managed_library(NAME stb CALL library_stb_init ORDER_INDEPENDENT ON)