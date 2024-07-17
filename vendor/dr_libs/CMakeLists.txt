function(library_dr_libs_init)

    cmake_minimum_required(VERSION 3.27.7)
    include(FetchContent)

    name_to_external_alias(alias "drlibs")

    # Include Sub-Project
    get_external_include_path(vendor_inc_path "dr_libs")
    get_external_source_path(vendor_src_path "dr_libs")

    CachedFetchContent_Declare(
            drlibs
            GIT_REPOSITORY https://github.com/mackron/dr_libs.git
            GIT_SUBMODULES_RECURSE OFF
            GIT_SHALLOW ON
            SOURCE_DIR "${vendor_src_path}"
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(
            drlibs
    )

    # Setup Library
    install_includes(
            SOURCE "${vendor_src_path}/dr_flac.h" "${vendor_src_path}/dr_wav.h"
            DESTINATION "${vendor_inc_path}/dr_libs/"
    )

endfunction()

# Push to setup
add_managed_library(NAME drlibs CALL library_dr_libs_init ORDER_INDEPENDENT ON)