function(library_spng_init)

	cmake_minimum_required(VERSION 3.25.1)
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
	get_external_include_path(vendor_inc_path "spng")
	get_external_source_path(vendor_src_path "spng")

	CachedFetchContent_Declare(
			libspng
			GIT_REPOSITORY https://github.com/randy408/libspng.git
			GIT_TAG adc94393dbeddf9e027d1b2dfff7c1bab975224e
			GIT_SUBMODULES_RECURSE FALSE
			GIT_SHALLOW FALSE
			GIT_PROGRESS TRUE
			SOURCE_DIR "${vendor_src_path}"
			PATCH_COMMAND git restore CMakeLists.txt
			COMMAND git apply ../spng.patch --ignore-space-change
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			libspng
	)

	# Setup Library
	add_library(${PROJECT_NAME}::${alias} ALIAS spng_static)

	install_includes(
			SOURCE "${vendor_src_path}/spng/spng.h"
			DESTINATION "${vendor_inc_path}/spng/"
	)

	export(TARGETS spng_static NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)

	# Deployment
	get_external_lib_path(vendor_dist_path "spng")
	set_batch_target_properties(spng_static ${vendor_dist_path})

endfunction()

# Push to setup
add_managed_library(NAME libspng CALL library_spng_init DEPS libzlib)