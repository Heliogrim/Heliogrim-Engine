function(library_ryml_init)

    cmake_minimum_required(VERSION 3.27.7)
    include(FetchContent)

    name_to_external_alias(c4core_alias "c4core")
    name_to_external_alias(ryml_alias "ryml")

    #
    set(RYML_WITH_TAB_TOKENS OFF CACHE INTERNAL "" FORCE)
    set(RYML_DEFAULT_CALLBACKS ON CACHE INTERNAL "" FORCE)
    set(RYML_BUILD_TOOLS OFF CACHE INTERNAL "" FORCE)
    set(RYML_BUILD_API OFF CACHE INTERNAL "" FORCE)
    set(RYML_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
    set(RYML_BUILD_BENCHMARKS OFF CACHE INTERNAL "" FORCE)
    set(RYML_DBG OFF CACHE INTERNAL "" FORCE)

    # Include Sub-Project
    get_external_include_path(lib_inc_path "ryml")
    get_external_source_path(lib_src_path "ryml")

    CachedFetchContent_Declare(
            rapidyaml
            GIT_REPOSITORY https://github.com/biojppm/rapidyaml.git
            GIT_TAG v0.5.0
            GIT_SHALLOW ON
            GIT_PROGRESS ON
            SOURCE_DIR "${lib_src_path}"
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(
            rapidyaml
    )

    # Setup Library
    add_library(${PROJECT_NAME}::${c4core_alias} ALIAS c4core)
    add_library(${PROJECT_NAME}::${ryml_alias} ALIAS ryml)

    install_includes(
            DIRECTORY "${lib_src_path}/src/"
            DESTINATION "${lib_inc_path}/ryml"
            INCLUDE_REGEX "^.+(h|hpp)$"
    )

    #export(TARGETS c4core NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${c4core_alias}/${c4core_alias}-export.cmake)
    #export(TARGETS ryml NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${ryml_alias}/${ryml_alias}-export.cmake)

    # Deployment
    get_external_lib_path(lib_dist_path "ryml")
    foreach (target c4core ryml)
        set_batch_target_properties(${target} ${lib_dist_path})
    endforeach ()

endfunction()

# Push to setup
add_managed_library(NAME rapidyaml CALL library_ryml_init ORDER_INDEPENDENT ON)