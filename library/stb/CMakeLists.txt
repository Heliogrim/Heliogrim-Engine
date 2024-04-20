function(library_stb_init)

	cmake_minimum_required(VERSION 3.27.7)
	include(FetchContent)

	name_to_external_alias(alias "stb")

	# Include Sub-Project
	get_external_include_path(lib_inc_path "stb")
	get_external_source_path(lib_src_path "stb")

	FetchContent_Declare(
			stb
			GIT_REPOSITORY https://github.com/nothings/stb.git
			GIT_TAG master
			GIT_SUBMODULES_RECURSE OFF
			GIT_SHALLOW ON
			GIT_PROGRESS ON
			SOURCE_DIR "${lib_src_path}"
			PATCH_COMMAND ${CMAKE_COMMAND} -E rm -f -- "${lib_src_path}/CMakeLists.txt"
			LOG_DOWNLOAD ON
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			stb
	)

	# Setup Library
	file(GLOB headers LIST_DIRECTORIES false "${lib_src_path}/*.*")
	list(FILTER headers INCLUDE REGEX "stb_.+\.(h|c)")
	install_includes(
			SOURCE "${headers}"
			DESTINATION "${lib_inc_path}/stb/"
	)

endfunction()

# Push to setup
add_managed_library(NAME stb CALL library_stb_init ORDER_INDEPENDENT ON)