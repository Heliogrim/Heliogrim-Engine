function(library_sdl_init)

    cmake_minimum_required(VERSION 3.27.7)
    include(FetchContent)

    name_to_external_alias(alias "sdl")

    #
    set(SDL_DISABLE_INSTALL ON CACHE INTERNAL "" FORCE)
    set(SDL_DISABLE_UNINSTALL ON CACHE INTERNAL "" FORCE)

    set(SDL_OPENGL OFF CACHE INTERNAL "" FORCE)
    set(SDL_OPENGLES OFF CACHE INTERNAL "" FORCE)
    set(SDL_X11 OFF CACHE INTERNAL "" FORCE)
    set(SDL_WAYLAND OFF CACHE INTERNAL "" FORCE)
    set(SDL_RPI OFF CACHE INTERNAL "" FORCE)
    set(SDL_ROCKCHIP OFF CACHE INTERNAL "" FORCE)
    set(SDL_COCOA OFF CACHE INTERNAL "" FORCE)
    set(SDL_DIRECTX OFF CACHE INTERNAL "" FORCE)
    set(SDL_XINPUT OFF CACHE INTERNAL "" FORCE)
    set(SDL_RENDER_D3D OFF CACHE INTERNAL "" FORCE)
    set(SDL_RENDER_METAL OFF CACHE INTERNAL "" FORCE)
    set(SDL_VIVANTE OFF CACHE INTERNAL "" FORCE)
    set(SDL_METAL OFF CACHE INTERNAL "" FORCE)
    set(SDL_KMSDRM OFF CACHE INTERNAL "" FORCE)
    set(SDL_OFFSCREEN OFF CACHE INTERNAL "" FORCE)
    set(SDL_DUMMYCAMERA OFF CACHE INTERNAL "" FORCE)
    set(SDL_HIDAPI OFF CACHE INTERNAL "" FORCE)
    set(SDL_HIDAPI_LIBUSB OFF CACHE INTERNAL "" FORCE)
    set(SDL_VIRTUAL_JOYSTICK OFF CACHE INTERNAL "" FORCE)
    set(SDL_HAPTIC OFF CACHE INTERNAL "" FORCE)
    set(SDL_JOYSTICK OFF CACHE INTERNAL "" FORCE)

    set(SDL_LIBUDEV OFF CACHE INTERNAL "" FORCE)
    set(SDL_ASAN OFF CACHE INTERNAL "" FORCE)
    set(SDL_CLANG_TIDY OFF CACHE INTERNAL "" FORCE)
    set(SDL_CCACHE ON CACHE INTERNAL "" FORCE)

    set(SDL_VULKAN ON CACHE INTERNAL "" FORCE)
    set(SDL_RENDER_VULKAN ON CACHE INTERNAL "" FORCE)

    set(SDL_SHARED ON CACHE INTERNAL "" FORCE)
    set(SDL_TEST OFF CACHE INTERNAL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE INTERNAL "" FORCE)
    set(SDL2_DISABLE_SDL2MAIN ON CACHE INTERNAL "" FORCE)

    # Include Sub-Project
    get_external_include_path(lib_inc_path "sdl")
    get_external_source_path(lib_src_path "sdl")

    CachedFetchContent_Declare(
            libsdl
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-2.30.2
            GIT_SHALLOW ON
            GIT_PROGRESS ON
            SOURCE_DIR "${lib_src_path}"
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(
            libsdl
    )

    # Setup Library
    add_library(${PROJECT_NAME}::${alias} ALIAS SDL2)

    install_includes(
            DIRECTORY "${lib_src_path}/include/"
            DESTINATION "${lib_inc_path}/sdl/"
    )

    export(TARGETS SDL2 NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)

    # Deployment
    get_external_lib_path(lib_dist_path "sdl")
    set_batch_target_properties(SDL2 ${lib_dist_path})

endfunction()

# Push to setup
add_managed_library(NAME libsdl CALL library_sdl_init ORDER_INDEPENDENT ON)