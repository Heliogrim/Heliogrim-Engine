function(library_ogg_init)

	cmake_minimum_required(VERSION 3.27.7)
	include(FetchContent)

	name_to_external_alias(alias "ogg")

	#
	set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
	set(BUILD_FRAMEWORK OFF CACHE INTERNAL "" FORCE)
	set(BUILD_TESTING OFF CACHE INTERNAL "" FORCE)
	set(INSTALL_DOCS OFF CACHE INTERNAL "" FORCE)
	set(INSTALL_PKG_CONFIG_MODULE OFF CACHE INTERNAL "" FORCE)
	set(INSTALL_CMAKE_PACKAGE_MODULE OFF CACHE INTERNAL "" FORCE)

	# Include Sub-Project
	get_external_include_path(lib_inc_path "ogg")
	get_external_source_path(lib_src_path "ogg")

	FetchContent_Declare(
			libogg
			GIT_REPOSITORY https://github.com/xiph/ogg.git
			GIT_TAG v1.3.5
			GIT_SHALLOW ON
			SOURCE_DIR "${lib_src_path}"
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			libogg
	)

	# Setup Library
	add_library(${PROJECT_NAME}::${alias} ALIAS ogg)

	install_includes(
			TARGET ogg
			TARGET_SOURCE "${lib_src_path}"
			DESTINATION "${lib_inc_path}/ogg"
	)

	#export(TARGETS ogg NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)

	# Deployment
	get_external_lib_path(lib_dist_path "ogg")
	set_batch_target_properties(ogg ${lib_dist_path})

endfunction()

# Push to setup
add_managed_library(NAME libogg CALL library_ogg_init ORDER_INDEPENDENT ON)