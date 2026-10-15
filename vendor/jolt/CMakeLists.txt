function(library_jolt_init)

	cmake_minimum_required(VERSION 3.25.1)
	include(FetchContent)

	name_to_external_alias(alias "jolt")

	#

	set(__USE_SHARED_LINKING ON)
	set(__USE_STATIC_LINKING OFF)
	if (DEFINED HELIOGRIM_FULL_STATIC_LINKING AND ${HELIOGRIM_FULL_STATIC_LINKING})
		set(__USE_SHARED_LINKING OFF)
		set(__USE_STATIC_LINKING ON)
	endif ()

	#

	propagate_option(VAR "OBJECT_LAYER_BITS" SET 16)
	propagate_option(VAR "CROSS_PLATFORM_DETERMINISTIC" SET OFF)

	propagate_option(VAR "CROSS_COMPILE_ARM" SET OFF)
	propagate_option(VAR "CPP_EXCEPTIONS_ENABLED" SET OFF)
	propagate_option(VAR "CPP_RTTI_ENABLED" SET OFF)
	propagate_option(VAR "ENABLE_ALL_WARNINGS" SET OFF)

	propagate_option(VAR "BUILD_SHARED_LIBS" SET ${__USE_SHARED_LINKING})
	propagate_option(VAR "ENABLE_INSTALL" SET OFF)
	propagate_option(VAR "JPH_ENABLE_VULKAN" SET ON)

	# Include Sub-Project
	get_external_include_path(vendor_inc_path "jolt")
	get_external_source_path(vendor_src_path "jolt")

	CachedFetchContent_Declare(
			jolt
			GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics"
			GIT_SUBMODULES_RECURSE FALSE
			GIT_SHALLOW TRUE
			GIT_TAG "v5.5.0"
			SOURCE_DIR "${vendor_src_path}"
			SOURCE_SUBDIR "Build"
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			jolt
	)

	# Setup Library
	add_library(${PROJECT_NAME}::${alias} ALIAS Jolt)

	install_includes(
			DIRECTORY "${vendor_src_path}/Jolt/"
			DESTINATION "${vendor_inc_path}/Jolt/"
			INCLUDE_REGEX "^.+(h|inl)$"
	)

	export(TARGETS Jolt ${__EXPOSED_AUX_TARGETS} NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${alias}/${alias}-export.cmake)

	# Deployment
	get_external_lib_path(vendor_dist_path "jolt")
	set_batch_target_properties(Jolt ${vendor_dist_path})

endfunction()

# Push to setup
add_managed_library(NAME jolt CALL library_jolt_init ORDER_INDEPENDENT ON)