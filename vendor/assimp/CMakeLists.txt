function(library_assimp_init)

	cmake_minimum_required(VERSION 3.25.1)
	include(FetchContent)

	name_to_external_alias(alias "assimp")

	#

	set(__USE_SHARED_LINKING ON)
	if (DEFINED HELIOGRIM_FULL_STATIC_LINKING AND ${HELIOGRIM_FULL_STATIC_LINKING})
		set(__USE_SHARED_LINKING OFF)
	endif ()

	#
	set(ASSIMP_HUNTER_ENABLED OFF CACHE INTERNAL "" FORCE)
	set(BUILD_SHARED_LIBS ${__USE_SHARED_LINKING} CACHE INTERNAL "" FORCE)
	set(ASSIMP_BUILD_FRAMEWORK OFF CACHE INTERNAL "" FORCE)
	set(ASSIMP_DOUBLE_PRECISION OFF CACHE INTERNAL "" FORCE)
	set(ASSIMP_OPT_BUILD_PACKAGES OFF CACHE INTERNAL "" FORCE)
	set(ASSIMP_ANDROID_JNIIOSYSTEM OFF CACHE INTERNAL "" FORCE)

	set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "" FORCE)
	set(ASSIMP_INSTALL OFF CACHE INTERNAL "" FORCE)
	set(ASSIMP_INSTALL_PDB OFF CACHE INTERNAL "" FORCE)

	set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "" FORCE)
	set(ASSIMP_BUILD_SAMPLES OFF CACHE INTERNAL "" FORCE)
	set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
	set(ASSIMP_COVERALLS OFF CACHE INTERNAL "" FORCE)

	set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE INTERNAL "" FORCE)
	set(ASSIMP_ASAN OFF CACHE INTERNAL "" FORCE)
	set(ASSIMP_UBSAN OFF CACHE INTERNAL "" FORCE)
	set(ASSIMP_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
	set(ASSIMP_BUILD_ZLIB OFF CACHE INTERNAL "" FORCE)
	set(USE_STATIC_CRT OFF CACHE INTERNAL "" FORCE)

	get_external_include_path(zlib_include_path "zlib")
	set(ZLIB_INCLUDE_DIR "${zlib_include_path}/zlib" CACHE INTERNAL "" FORCE)
	set(ZLIB_LIBRARY ${PROJECT_NAME}::External-zlib CACHE INTERNAL "" FORCE)
	set(ZLIB_LIBRARIES ${PROJECT_NAME}::External-zlib CACHE INTERNAL "" FORCE)
	set(ZLIB_FOUND ON CACHE INTERNAL "" FORCE)

	# Include Sub-Project
	get_vendor_path(vendor_base_path)
	get_external_include_path(vendor_inc_path "assimp")
	get_external_source_path(vendor_src_path "assimp")

	CachedFetchContent_Declare(
			libassimp
			GIT_REPOSITORY https://github.com/assimp/assimp.git
			GIT_TAG v5.4.0
			GIT_SHALLOW TRUE
			GIT_PROGRESS TRUE
			SOURCE_DIR "${vendor_src_path}"
			PATCH_COMMAND git restore CMakeLists.txt
			COMMAND git restore ./cmake-modules/FindZLIB.cmake
			COMMAND git apply ../assimp.patch --ignore-space-change
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			libassimp
	)

	# Setup Library
	add_library(${PROJECT_NAME}::${alias} ALIAS assimp)

	# Setup Library
	install_includes(
			DIRECTORY "${vendor_src_path}/include/"
			DESTINATION "${vendor_inc_path}/"
	)

	export(TARGETS assimp NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)

	# Deployment
	get_external_lib_path(vendor_dist_path "assimp")
	set_batch_target_properties(assimp ${vendor_dist_path})

	# Reset Settings
	set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)

endfunction()

# Push to setup
add_managed_library(NAME libassimp CALL library_assimp_init DEPS libzlib)