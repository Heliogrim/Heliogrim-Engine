function(library_glm_init)

    cmake_minimum_required(VERSION 3.25.1)
    include(FetchContent)

    name_to_external_alias(alias "glm")

    # Include Sub-Project
    get_external_include_path(vendor_inc_path "glm")
    get_external_source_path(vendor_src_path "glm")

    CachedFetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG 1.0.1
            GIT_SUBMODULES_RECURSE FALSE
            GIT_SHALLOW TRUE
            SOURCE_DIR "${vendor_src_path}"
            PATCH_COMMAND ${CMAKE_COMMAND} -E rm -f -- "${vendor_src_path}/CMakeLists.txt"
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(
            glm
    )

    # Setup Library
    install_includes(
            DIRECTORY "${vendor_src_path}/glm/"
            DESTINATION "${vendor_inc_path}/glm/"
    )

endfunction()

# Push to setup
add_managed_library(NAME glm CALL library_glm_init ORDER_INDEPENDENT ON)