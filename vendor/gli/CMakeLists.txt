function(library_gli_init)

	cmake_minimum_required(VERSION 3.25.1)
	include(FetchContent)

	name_to_external_alias(alias "gli")

	# Include Sub-Project
	get_external_include_path(vendor_inc_path "gli")
	get_external_source_path(vendor_src_path "gli")

	CachedFetchContent_Declare(
			gli
			GIT_REPOSITORY https://github.com/g-truc/gli.git
			GIT_TAG master
			GIT_SUBMODULES_RECURSE FALSE
			GIT_SHALLOW TRUE
			GIT_PROGRESS TRUE
			SOURCE_DIR "${vendor_src_path}"
			PATCH_COMMAND ${CMAKE_COMMAND} -E rm -f -- "${vendor_src_path}/CMakeLists.txt"
			COMMAND git restore gli/core/save_dds.inl
			COMMAND git apply ../gli.patch --ignore-space-change
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			gli
	)

	# Setup Library
	install_includes(
			DIRECTORY "${vendor_src_path}/gli/"
			DESTINATION "${vendor_inc_path}/gli/"
	)

endfunction()

# Push to setup
add_managed_library(NAME gli CALL library_gli_init ORDER_INDEPENDENT ON)