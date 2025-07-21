function(library_freetype_init)

    cmake_minimum_required(VERSION 3.25.1)
    include(FetchContent)

    name_to_external_alias(alias "freetype")

    #
    set(FT_DISABLE_ZLIB OFF CACHE INTERNAL "" FORCE)
    set(FT_DISABLE_BZIP2 ON CACHE INTERNAL "" FORCE)
    set(FT_DISABLE_PNG ON CACHE INTERNAL "" FORCE)
    set(FT_DISABLE_HARFBUZZ ON CACHE INTERNAL "" FORCE)
    set(FT_DISABLE_BROTLI ON CACHE INTERNAL "" FORCE)

    set(FT_ENABLE_ERROR_STRINGS OFF CACHE INTERNAL "" FORCE)

    set(FT_REQUIRE_BROTLI OFF CACHE INTERNAL "" FORCE)
    set(FT_REQUIRE_BZIP2 OFF CACHE INTERNAL "" FORCE)
    set(FT_REQUIRE_HARFBUZZ OFF CACHE INTERNAL "" FORCE)
    set(FT_REQUIRE_PNG OFF CACHE INTERNAL "" FORCE)
    set(FT_REQUIRE_ZLIB ON CACHE INTERNAL "" FORCE)

    get_external_include_path(zlib_include_path "zlib")
    set(ZLIB_INCLUDE_DIR "${zlib_include_path}/zlib" CACHE INTERNAL "" FORCE)
    set(ZLIB_LIBRARY ${PROJECT_NAME}::External-zlib CACHE INTERNAL "" FORCE)
    set(ZLIB_LIBRARIES ${PROJECT_NAME}::External-zlib CACHE INTERNAL "" FORCE)
    set(ZLIB_FOUND ON CACHE INTERNAL "" FORCE)

    # Include Sub-Project
    get_external_include_path(vendor_inc_path "freetype2")
    get_external_source_path(vendor_src_path "freetype2")

    CachedFetchContent_Declare(
            libfreetype
            GIT_REPOSITORY https://github.com/freetype/freetype.git
            GIT_TAG VER-2-13-3
            GIT_SHALLOW ON
            GIT_PROGRESS ON
            SOURCE_DIR "${vendor_src_path}"
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(
            libfreetype
    )

    # Setup Library
    add_library(${PROJECT_NAME}::${alias} ALIAS freetype)

    install_includes(
            DIRECTORY "${vendor_src_path}/include/"
            DESTINATION "${vendor_inc_path}/"
    )

    export(TARGETS freetype NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)

    # Deployment
    get_external_lib_path(vendor_dist_path "freetype2")
    set_batch_target_properties(freetype ${vendor_dist_path})

endfunction()

# Push to setup
add_managed_library(NAME freetype CALL library_freetype_init DEPS libzlib)