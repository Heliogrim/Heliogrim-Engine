function(library_nontype_fn_init)

	cmake_minimum_required(VERSION 3.25.1)
	include(FetchContent)

	name_to_external_alias(alias "nontype_fn")

	# Include Sub-Project
	get_external_include_path(vendor_inc_path "nontype_fn")
	get_external_source_path(vendor_src_path "nontype_fn")

	CachedFetchContent_Declare(
			nontype_fn
			GIT_REPOSITORY https://github.com/zhihaoy/nontype_functional.git
			GIT_TAG v1.0.2
			GIT_SUBMODULES_RECURSE FALSE
			GIT_SHALLOW TRUE
			SOURCE_DIR "${vendor_src_path}"
			PATCH_COMMAND ${CMAKE_COMMAND} -E rm -f -- "${vendor_src_path}/CMakeLists.txt"
			EXCLUDE_FROM_ALL
			OVERRIDE_FIND_PACKAGE
	)
	FetchContent_MakeAvailable(
			nontype_fn
	)

	# Setup Library
	install_includes(
			DIRECTORY "${vendor_src_path}/include/"
			DESTINATION "${vendor_inc_path}/"
	)

endfunction()

# Push to setup
add_managed_library(NAME nontype_fn CALL library_nontype_fn_init ORDER_INDEPENDENT ON)