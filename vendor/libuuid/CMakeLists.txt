function(library_libuuid_init)

	cmake_minimum_required(VERSION 3.27.7)
	include(FetchContent)

	name_to_external_alias(alias "libuuid")

	#
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
	set(LIBUUID_SHARED OFF CACHE BOOL "" FORCE)
	set(LIBUUID_STATIC ON CACHE BOOL "" FORCE)
	set(LIBUUID_RUNSTATEDIR "/run" CACHE PATH "" FORCE)
	set(LIBUUID_LOCALSTATEDIR "/var" CACHE PATH "" FORCE)

	# Include Sub-Project
	get_external_include_path(vendor_inc_path "libuuid")
	get_external_source_path(vendor_src_path "libuuid")

	CachedFetchContent_Declare(
			libuuid
			GIT_REPOSITORY https://github.com/gershnik/libuuid-cmake.git
			GIT_TAG v2.40.2
			GIT_SUBMODULES_RECURSE FALSE
			GIT_SHALLOW TRUE
			SOURCE_DIR "${vendor_src_path}"
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			libuuid
	)

	# Setup Library
	add_library(${PROJECT_NAME}::${alias} ALIAS uuid_static)

	#install_includes(
	#		TARGET uuid_header
	#		TARGET_SOURCE "."
	#		DESTINATION "${vendor_inc_path}/uuid/"
	#)

	export(TARGETS uuid_static uuid_header NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)

	# Deployment
	get_external_lib_path(vendor_dist_path "libuuid")
	foreach (target uuid_static)
		set_batch_target_properties(${target} ${vendor_dist_path})
	endforeach ()

endfunction()

# Push to setup
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	add_managed_library(NAME libuuid CALL library_libuuid_init ORDER_INDEPENDENT ON)
endif ()