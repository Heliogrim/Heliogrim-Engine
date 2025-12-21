function(library_vulkan_headers_init)

	cmake_minimum_required(VERSION 3.25.1)
	include(FetchContent)

	name_to_external_alias(alias "vulkan-headers")

	#
	set(VULKAN_HEADERS_ENABLE_TESTS OFF CACHE INTERNAL "" FORCE)
	set(VULKAN_HEADERS_ENABLE_INSTALL OFF CACHE INTERNAL "" FORCE)
	set(VULKAN_HEADERS_ENABLE_MODULE OFF CACHE INTERNAL "" FORCE)

	# Include Sub-Project
	get_external_include_path(vendor_inc_path "vulkan-headers")
	get_external_source_path(vendor_src_path "vulkan-headers")

	CachedFetchContent_Declare(
			vulkan-headers
			GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
			GIT_TAG vulkan-sdk-1.4.335.0
			GIT_SUBMODULES_RECURSE OFF
			GIT_SHALLOW ON
			SOURCE_DIR "${vendor_src_path}"
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			vulkan-headers
	)

	# Setup Library
	install_includes(
			DIRECTORY "${vendor_src_path}/include/"
			DESTINATION "${vendor_inc_path}/"
	)

	export(TARGETS Vulkan-Headers NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)
	export(TARGETS Vulkan-Headers NAMESPACE Vulkan:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/VulkanHeadersConfig.cmake)

endfunction()

# Push to setup
add_managed_library(NAME vulkan-headers CALL library_vulkan_headers_init ORDER_INDEPENDENT ON)