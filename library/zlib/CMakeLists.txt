function(library_zlib_init)

	cmake_minimum_required(VERSION 3.27.7)
	include(FetchContent)

	name_to_external_alias(alias "zlib")

	#
	set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
	set(ZLIB_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
	set(SKIP_INSTALL_ALL ON CACHE INTERNAL "" FORCE)

	# Include Sub-Project
	get_external_lib_path(lib_lib_path "zlib")
	get_external_include_path(lib_inc_path "zlib")
	get_external_source_path(lib_src_path "zlib")

	FetchContent_Declare(
			libzlib
			GIT_REPOSITORY https://github.com/madler/zlib.git
			GIT_TAG v1.3.1
			SOURCE_DIR "${lib_src_path}"
			PATCH_COMMAND git restore CMakeLists.txt
			COMMAND git apply ../zlib.patch
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			libzlib
	)

	# Setup Library
	add_library(${PROJECT_NAME}::${alias} ALIAS zlibstatic)

	get_target_property(zlib_build_path zlibstatic BINARY_DIR)
	file(INSTALL "${lib_src_path}/zlib.h" DESTINATION "${lib_inc_path}/zlib/")
	file(INSTALL "${zlib_build_path}/zconf.h" DESTINATION "${lib_inc_path}/zlib/")

	export(TARGETS zlibstatic NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)

	# Deployment
	get_external_lib_path(lib_dist_path "zlib")
	foreach (target zlibstatic zlib)
		set_batch_target_properties(${target} ${lib_dist_path})
	endforeach ()

endfunction()

# Push to setup
add_managed_library(NAME libzlib CALL library_zlib_init ORDER_INDEPENDENT ON)