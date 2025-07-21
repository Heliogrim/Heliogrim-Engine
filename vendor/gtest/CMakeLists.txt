function(library_gtest_init)

    cmake_minimum_required(VERSION 3.25.1)
    include(FetchContent)

    name_to_external_alias(gtest_alias "gtest")
    name_to_external_alias(gtest_main_alias "gtest_main")
    name_to_external_alias(gmock_alias "gmock")
    name_to_external_alias(gmock_main_alias "gmock_main")

    #
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)

    set(BUILD_GMOCK ON CACHE INTERNAL "" FORCE)
    set(INSTALL_GTEST OFF CACHE INTERNAL "" FORCE)
    set(GTEST_HAS_ABSL OFF CACHE INTERNAL "" FORCE)

    set(gtest_force_shared_crt OFF CACHE INTERNAL "" FORCE)
    set(gtest_build_tests OFF CACHE INTERNAL "" FORCE)
    set(gtest_build_samples OFF CACHE INTERNAL "" FORCE)
    set(gtest_disable_pthreads OFF CACHE INTERNAL "" FORCE)
    set(gtest_hide_internal_symbols OFF CACHE INTERNAL "" FORCE)

    set(gmock_build_tests OFF CACHE INTERNAL "" FORCE)

    # Include Sub-Project
    get_external_source_path(vendor_src_path "gtest")
    get_external_include_path(vendor_inc_path "gtest")

    CachedFetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.17.0
            GIT_SHALLOW ON
            SOURCE_DIR "${vendor_src_path}"
            EXCLUDE_FROM_ALL
            OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(
            googletest
    )

    # Setup Library
    add_library(${PROJECT_NAME}::${gtest_alias} ALIAS gtest)
    add_library(${PROJECT_NAME}::${gtest_main_alias} ALIAS gtest_main)
    add_library(${PROJECT_NAME}::${gmock_alias} ALIAS gmock)
    add_library(${PROJECT_NAME}::${gmock_main_alias} ALIAS gmock_main)

    install_includes(
            DIRECTORY "${vendor_src_path}/googletest/include/" "${vendor_src_path}/googlemock/include/"
            DESTINATION "${vendor_inc_path}/"
    )

    export(TARGETS gtest NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${gtest_alias}/${gtest_alias}-export.cmake)
    export(TARGETS gtest_main NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${gtest_main_alias}/${gtest_main_alias}-export.cmake)
    export(TARGETS gmock NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${gmock_alias}/${gmock_alias}-export.cmake)
    export(TARGETS gmock_main NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${gmock_main_alias}/${gmock_main_alias}-export.cmake)

    # Deployment
    get_external_lib_path(vendor_dist_path "gtest")
    foreach (target gtest gtest_main gmock gmock_main)
        set_batch_target_properties(${target} ${vendor_dist_path})
    endforeach ()

endfunction()

# Push to setup
add_managed_library(
        NAME googletest
        CALL library_gtest_init
        ORDER_INDEPENDENT ON
        TARGETS gtest gtest_main gmock gmock_main
)