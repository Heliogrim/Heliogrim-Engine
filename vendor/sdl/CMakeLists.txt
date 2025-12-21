function(library_sdl_init)

	cmake_minimum_required(VERSION 3.25.1)
	include(FetchContent)

	name_to_external_alias(alias "sdl")

	#

	set(__USE_SHARED_LINKING ON)
	set(__USE_STATIC_LINKING OFF)
	if (DEFINED HELIOGRIM_FULL_STATIC_LINKING AND ${HELIOGRIM_FULL_STATIC_LINKING})
		set(__USE_SHARED_LINKING OFF)
		set(__USE_STATIC_LINKING ON)
	endif ()

	#
	propagate_option(VAR "SDL_DISABLE_INSTALL" SET ON)
	propagate_option(VAR "SDL_DISABLE_UNINSTALL" SET ON)
	propagate_option(VAR "SDL_INSTALL_CPACK" SET OFF)
	propagate_option(VAR "SDL_INSTALL_DOCS" SET OFF)
	propagate_option(VAR "SDL_UNINSTALL" SET OFF)

	propagate_option(VAR "SDL_OPENGL" SET OFF)
	propagate_option(VAR "SDL_OPENGLES" SET OFF)
	propagate_option(VAR "SDL_X11" SET ${HELIOGRIM_PLATFORM_X11})
	propagate_option(VAR "SDL_WAYLAND" SET ${HELIOGRIM_PLATFORM_WAYLAND})
	propagate_option(VAR "SDL_RPI" SET OFF)
	propagate_option(VAR "SDL_ROCKCHIP" SET OFF)
	propagate_option(VAR "SDL_COCOA" SET OFF)
	propagate_option(VAR "SDL_DIRECTX" SET OFF)
	propagate_option(VAR "SDL_XINPUT" SET OFF)
	propagate_option(VAR "SDL_RENDER_D3D" SET OFF)
	propagate_option(VAR "SDL_RENDER_METAL" SET OFF)
	propagate_option(VAR "SDL_VIVANTE" SET OFF)
	propagate_option(VAR "SDL_METAL" SET OFF)
	propagate_option(VAR "SDL_KMSDRM" SET OFF)
	propagate_option(VAR "SDL_OFFSCREEN" SET OFF)
	propagate_option(VAR "SDL_DUMMYCAMERA" SET OFF)
	propagate_option(VAR "SDL_HIDAPI" SET OFF)
	propagate_option(VAR "SDL_HIDAPI_LIBUSB" SET OFF)
	propagate_option(VAR "SDL_VIRTUAL_JOYSTICK" SET OFF)
	propagate_option(VAR "SDL_HAPTIC" SET OFF)
	propagate_option(VAR "SDL_JOYSTICK" SET OFF)

	propagate_option(VAR "SDL_LIBUDEV" SET OFF)
	propagate_option(VAR "SDL_ASAN" SET OFF)
	propagate_option(VAR "SDL_CLANG_TIDY" SET OFF)
	propagate_option(VAR "SDL_CCACHE" SET ON)

	propagate_option(VAR "SDL_VULKAN" SET ON)
	propagate_option(VAR "SDL_RENDER_VULKAN" SET ON)

	propagate_option(VAR "SDL_SHARED" SET ${__USE_SHARED_LINKING})
	propagate_option(VAR "SDL_STATIC" SET ${__USE_STATIC_LINKING})
	propagate_option(VAR "SDL_TESTS" SET OFF)
	propagate_option(VAR "SDL_TEST_LIBRARY" SET OFF)
	propagate_option(VAR "SDL_EXAMPLES" SET OFF)
	propagate_option(VAR "SDL3_MAINPROJECT" SET ON)

	# Include Sub-Project
	get_external_include_path(vendor_inc_path "sdl")
	get_external_source_path(vendor_src_path "sdl")

	CachedFetchContent_Declare(
			libsdl
			GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
			GIT_TAG release-3.2.22
			GIT_SHALLOW ON
			GIT_PROGRESS ON
			SOURCE_DIR "${vendor_src_path}"
			EXCLUDE_FROM_ALL
			OVERRIDE_FIND_PACKAGE
	)
	FetchContent_MakeAvailable(
			libsdl
	)

	# Resolve Export Target

	set(__EXPOSED_TARGET SDL3-shared)
	set(__EXPOSED_AUX_TARGETS)
	if (${__USE_STATIC_LINKING})
		set(__EXPOSED_TARGET SDL3-static)
		set(__EXPOSED_AUX_TARGETS sdl-build-options)
	endif ()

	# Setup Library
	add_library(${PROJECT_NAME}::${alias} ALIAS ${__EXPOSED_TARGET})

	install_includes(
			DIRECTORY "${vendor_src_path}/include/"
			DESTINATION "${vendor_inc_path}/sdl/"
	)

	#export(TARGETS ${__EXPOSED_TARGET} ${__EXPOSED_AUX_TARGETS} NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)

	# Deployment
	get_external_lib_path(vendor_dist_path "sdl")
	set_batch_target_properties(${__EXPOSED_TARGET} ${vendor_dist_path})

endfunction()

# Push to setup
add_managed_library(NAME libsdl CALL library_sdl_init ORDER_INDEPENDENT ON)