function(library_optional_init)

    cmake_minimum_required(VERSION 3.25.1)
    include(FetchContent)

    name_to_external_alias(alias "optional")

    # Include Sub-Project
    get_external_include_path(vendor_inc_path "optional")
    get_external_source_path(vendor_src_path "optional")

    CachedFetchContent_Declare(
            optional
            GIT_REPOSITORY https://github.com/TartanLlama/optional.git
            GIT_TAG v1.1.0
            GIT_SUBMODULES_RECURSE FALSE
            GIT_SHALLOW TRUE
            SOURCE_DIR "${vendor_src_path}"
            PATCH_COMMAND ${CMAKE_COMMAND} -E rm -f -- "${vendor_src_path}/CMakeLists.txt"
            EXCLUDE_FROM_ALL
            OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(
            optional
    )

    # Setup Library
    install_includes(
            DIRECTORY "${vendor_src_path}/include/"
            DESTINATION "${vendor_inc_path}/"
    )

endfunction()

# Push to setup
add_managed_library(NAME optional CALL library_optional_init ORDER_INDEPENDENT ON)