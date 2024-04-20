function(library_spng_init)

	cmake_minimum_required(VERSION 3.27.7)
	include(FetchContent)

	name_to_external_alias(alias "spng")

	#
	set(ENABLE_OPT ON CACHE BOOL "" FORCE)
	set(SPNG_SHARED OFF CACHE BOOL "" FORCE)
	set(SPNG_STATIC ON CACHE BOOL "" FORCE)
	set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

	get_external_include_path(zlib_include_path "zlib")
	set(ZLIB_INCLUDE_DIR "${zlib_include_path}/zlib" CACHE INTERNAL "" FORCE)
	set(ZLIB_LIBRARY ${PROJECT_NAME}::External-zlib CACHE INTERNAL "" FORCE)
	set(ZLIB_LIBRARIES ${PROJECT_NAME}::External-zlib CACHE INTERNAL "" FORCE)
	set(ZLIB_FOUND ON CACHE INTERNAL "" FORCE)

	# Include Sub-Project
	get_external_include_path(lib_inc_path "spng")
	get_external_source_path(lib_src_path "spng")

	FetchContent_Declare(
			libspng
			GIT_REPOSITORY https://github.com/randy408/libspng.git
			GIT_TAG master
			GIT_SUBMODULES_RECURSE OFF
			GIT_SHALLOW ON
			GIT_PROGRESS ON
			SOURCE_DIR "${lib_src_path}"
			PATCH_COMMAND git restore CMakeLists.txt
			COMMAND git apply ../spng.patch
			LOG_DOWNLOAD ON
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			libspng
	)

	# Setup Library
	add_library(${PROJECT_NAME}::${alias} ALIAS spng_static)

	install_includes(
			SOURCE "${lib_src_path}/spng/spng.h"
			DESTINATION "${lib_inc_path}/spng/"
	)

	export(TARGETS spng_static NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)

	# Deployment
	get_external_lib_path(lib_dist_path "spng")
	set_batch_target_properties(spng_static ${lib_dist_path})

endfunction()

# Push to setup
add_managed_library(NAME libspng CALL library_spng_init DEPS libzlib)