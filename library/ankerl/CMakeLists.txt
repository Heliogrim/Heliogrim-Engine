function(library_ankerl_init)

	cmake_minimum_required(VERSION 3.27.7)
	include(FetchContent)

	name_to_external_alias(alias "ankerl")

	# Include Sub-Project
	get_external_include_path(lib_inc_path "ankerl")
	get_external_source_path(lib_src_path "ankerl")

	FetchContent_Declare(
			ankerl
			GIT_REPOSITORY https://github.com/martinus/unordered_dense.git
			GIT_TAG v4.4.0
			GIT_SUBMODULES_RECURSE OFF
			GIT_SHALLOW ON
			SOURCE_DIR "${lib_src_path}"
			PATCH_COMMAND ${CMAKE_COMMAND} -E rm -f -- "${lib_src_path}/CMakeLists.txt"
			LOG_DOWNLOAD ON
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			ankerl
	)

	# Setup Library
	install_includes(
			DIRECTORY "${lib_src_path}/include/"
			DESTINATION "${lib_inc_path}/"
	)

endfunction()

# Push to setup

add_managed_library(NAME ankerl CALL library_ankerl_init ORDER_INDEPENDENT ON)