function(library_crcpp_init)

	cmake_minimum_required(VERSION 3.25.1)
	include(FetchContent)

	name_to_external_alias(alias "crcpp")

	#
	set(BUILD_DOC OFF CACHE INTERNAL "" FORCE)
	set(BUILD_TEST OFF CACHE INTERNAL "" FORCE)

	# Include Sub-Project
	get_external_include_path(vendor_inc_path "crcpp")
	get_external_source_path(vendor_src_path "crcpp")

	CachedFetchContent_Declare(
			crcpp
			GIT_REPOSITORY https://github.com/d-bahr/CRCpp.git
			GIT_TAG release-1.2.0.0
			GIT_SUBMODULES_RECURSE OFF
			GIT_SHALLOW ON
			SOURCE_DIR "${vendor_src_path}"
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			crcpp
	)

	# Setup Library
	install_includes(
			DIRECTORY "${vendor_src_path}/inc/"
			DESTINATION "${vendor_inc_path}/crcpp/"
	)

endfunction()

# Push to setup
add_managed_library(NAME crcpp CALL library_crcpp_init ORDER_INDEPENDENT ON)