function(library_tinyexr_init)

    cmake_minimum_required(VERSION 3.27.7)
    include(FetchContent)

    name_to_external_alias(alias "tinyexr")

    # Include Sub-Project
    get_external_include_path(vendor_inc_path "tinyexr")
    get_external_source_path(vendor_src_path "tinyexr")

    CachedFetchContent_Declare(
            tinyexr
            GIT_REPOSITORY https://github.com/syoyo/tinyexr.git
            GIT_TAG v1.0.8
            GIT_SUBMODULES_RECURSE OFF
            GIT_SHALLOW ON
            SOURCE_DIR "${vendor_src_path}"
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(
            tinyexr
    )

    # Setup Library
    install_includes(
            SOURCE "${vendor_src_path}/tinyexr.h"
            DESTINATION "${vendor_inc_path}/tinyexr/"
    )

endfunction()

# Push to setup
add_managed_library(NAME tinyexr CALL library_tinyexr_init ORDER_INDEPENDENT ON)