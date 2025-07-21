function(library_spdlog_init)

    cmake_minimum_required(VERSION 3.25.1)
    include(FetchContent)

    name_to_external_alias(alias "spdlog")

    #
    set(SPDLOG_BUILD_ALL OFF CACHE INTERNAL "" FORCE)
    set(SPDLOG_BUILD_SHARED OFF CACHE INTERNAL "" FORCE)
    set(SPDLOG_ENABLE_PCH OFF CACHE INTERNAL "" FORCE)

    set(SPDLOG_BUILD_EXAMPLE OFF CACHE INTERNAL "" FORCE)
    set(SPDLOG_BUILD_EXAMPLE_HO OFF CACHE INTERNAL "" FORCE)
    set(SPDLOG_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
    set(SPDLOG_BUILD_TESTS_HO OFF CACHE INTERNAL "" FORCE)
    set(SPDLOG_BUILD_BENCH OFF CACHE INTERNAL "" FORCE)

    set(SPDLOG_SANITIZE_ADDRESS OFF CACHE INTERNAL "" FORCE)
    set(SPDLOG_BUILD_WARNINGS OFF CACHE INTERNAL "" FORCE)

    set(SPDLOG_SYSTEM_INCLUDES OFF CACHE INTERNAL "" FORCE)
    set(SPDLOG_INSTALL OFF CACHE INTERNAL "" FORCE)
    set(SPDLOG_USE_STD_FORMAT ON CACHE INTERNAL "" FORCE)
    set(SPDLOG_FMT_EXTERNAL OFF CACHE INTERNAL "" FORCE)
    set(SPDLOG_FMT_EXTERNAL_HO OFF CACHE INTERNAL "" FORCE)
    set(SPDLOG_NO_EXCEPTIONS OFF CACHE INTERNAL "" FORCE)

    set(SPDLOG_NO_TLS OFF CACHE INTERNAL "" FORCE)

    # Include Sub-Project
    get_external_include_path(vendor_inc_path "spdlog")
    get_external_source_path(vendor_src_path "spdlog")

    CachedFetchContent_Declare(
            libspdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.15.3
            GIT_SHALLOW ON
            GIT_PROGRESS ON
            SOURCE_DIR "${vendor_src_path}"
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(
            libspdlog
    )

    # Setup Library
    add_library(${PROJECT_NAME}::${alias} ALIAS spdlog)

    install_includes(
            DIRECTORY "${vendor_src_path}/include/"
            DESTINATION "${vendor_inc_path}/"
            EXCLUDE_PATTERN "fmt/bundled"
    )

    export(TARGETS spdlog NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)

    # Deployment
    get_external_lib_path(vendor_dist_path "spdlog")
    set_batch_target_properties(spdlog ${vendor_dist_path})

endfunction()

# Push to setup
add_managed_library(NAME libspdlog CALL library_spdlog_init ORDER_INDEPENDENT ON)