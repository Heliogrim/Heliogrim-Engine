function(library_spirv_headers_init)

	cmake_minimum_required(VERSION 3.27.7)
	include(FetchContent)

	name_to_external_alias(headers_alias "spirv-headers")

	#
	set(SPIRV_HEADERS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
	set(SPIRV_HEADERS_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

	# Include Sub-Project
	get_external_include_path(lib_inc_path "spirv-headers")
	get_external_source_path(lib_src_path "spirv-headers")

	FetchContent_Declare(
			libspirv-headers
			GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
			GIT_TAG vulkan-sdk-1.3.280.0
			GIT_SHALLOW ON
			GIT_PROGRESS ON
			SOURCE_DIR "${lib_src_path}"
			LOG_DOWNLOAD ON
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			libspirv-headers
	)

	# Setup Library
	add_library(${PROJECT_NAME}::${headers_alias} ALIAS SPIRV-Headers)

	install_includes(
			DIRECTORY "${lib_src_path}/include/"
			DESTINATION "${lib_inc_path}/"
	)

	export(TARGETS SPIRV-Headers NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${headers_alias}/${headers_alias}-export.cmake)

	# Deployment
	get_external_lib_path(lib_dist_path "spirv-headers")
	foreach (target SPIRV-Headers)
		set_batch_target_properties(${target} ${lib_dist_path})
	endforeach ()

endfunction()

# Push to setup
add_managed_library(NAME libspirv-headers CALL library_spirv_headers_init ORDER_INDEPENDENT ON)