function(library_glm_init)

	cmake_minimum_required(VERSION 3.27.7)
	include(FetchContent)

	name_to_external_alias(alias "glm")

	# Include Sub-Project
	get_external_include_path(lib_inc_path "glm")
	get_external_source_path(lib_src_path "glm")

	FetchContent_Declare(
			glm
			GIT_REPOSITORY https://github.com/g-truc/glm.git
			GIT_TAG 1.0.1
			GIT_SUBMODULES_RECURSE OFF
			GIT_SHALLOW ON
			SOURCE_DIR "${lib_src_path}"
			PATCH_COMMAND ${CMAKE_COMMAND} -E rm -f -- "${lib_src_path}/CMakeLists.txt"
			CONFIGURE_COMMAND ""
			LOG_DOWNLOAD ON
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			glm
	)

	# Setup Library
	install_includes(
			DIRECTORY "${lib_src_path}/glm/"
			DESTINATION "${lib_inc_path}/glm/"
	)

endfunction()

# Push to setup
add_managed_library(NAME glm CALL library_glm_init ORDER_INDEPENDENT ON)