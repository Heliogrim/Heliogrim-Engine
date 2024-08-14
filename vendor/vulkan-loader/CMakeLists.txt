function(library_vulkan_loader_init)

	cmake_minimum_required(VERSION 3.25.1)
	include(FetchContent)

	name_to_external_alias(alias "vulkan-loader")

	#
	set(BUILD_WSI_XCB_SUPPORT ${HELIOGRIM_PLATFORM_X11} CACHE INTERNAL "" FORCE)
	set(BUILD_WSI_XLIB_SUPPORT ${HELIOGRIM_PLATFORM_X11} CACHE INTERNAL "" FORCE)
	set(BUILD_WSI_WAYLAND_SUPPORT ${HELIOGRIM_PLATFORM_WAYLAND} CACHE INTERNAL "" FORCE)

	#
	get_external_include_path(vulkan_headers_include_path "vulkan-headers")
	name_to_external_alias(vulkan_header_alias "vulkan-headers")
	set(VULKAN_HEADERS_INSTALL_DIR "${vulkan_headers_include_path}/" CACHE INTERNAL "" FORCE)
	set(VulkanHeaders_FOUND ON CACHE INTERNAL "" FORCE)
	set(VulkanHeaders_CONFIG "${PROJECT_BINARY_DIR}/cmake/${vulkan_header_alias}/VulkanHeadersConfig.cmake" CACHE INTERNAL "" FORCE)

	# Include Sub-Project
	get_external_include_path(vendor_inc_path "vulkan-loader")
	get_external_source_path(vendor_src_path "vulkan-loader")

	CachedFetchContent_Declare(
			vulkan-loader
			GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
			GIT_TAG v1.3.290
			GIT_SHALLOW ON
			GIT_PROGRESS ON
			SOURCE_DIR "${vendor_src_path}"
			PATCH_COMMAND git restore CMakeLists.txt
			COMMAND git apply ../vulkan-loader.patch --ignore-space-change
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			vulkan-loader
	)

	# Setup Library
	add_library(${PROJECT_NAME}::${alias} ALIAS vulkan)

	export(TARGETS vulkan NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)

	# Deployment
	get_external_lib_path(vendor_dist_path "vulkan-loader")
	set_batch_target_properties(vulkan ${vendor_dist_path})

endfunction()

# Push to setup
add_managed_library(NAME vulkan-loader CALL library_vulkan_loader_init DEPS vulkan-headers)