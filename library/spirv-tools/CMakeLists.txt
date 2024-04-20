function(library_spirv_tools_init)

	cmake_minimum_required(VERSION 3.27.7)
	include(FetchContent)

	name_to_external_alias(static_alias "spirv-static")
	name_to_external_alias(reduce_alias "spirv-reduce")
	name_to_external_alias(opt_alias "spirv-opt")
	name_to_external_alias(link_alias "spirv-link")
	name_to_external_alias(lint_alias "spirv-lint")
	name_to_external_alias(diff_alias "spirv-diff")

	#
	set(ENABLE_RTTI OFF CACHE BOOL "" FORCE)
	set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
	set(ENABLE_SPIRV_TOOLS_INSTALL OFF CACHE BOOL "" FORCE)

	set(SPIRV_BUILD_FUZZER OFF CACHE BOOL "" FORCE)
	set(SPIRV_BUILD_LIBFUZZER_TARGETS OFF CACHE BOOL "" FORCE)
	set(SPIRV_WARN_EVERYTHING OFF CACHE BOOL "" FORCE)
	set(SPIRV_WERROR OFF CACHE BOOL "" FORCE)
	set(SPIRV_COLOR_TERMINAL ON CACHE BOOL "" FORCE)
	set(SPIRV_LOG_DEBUG OFF CACHE BOOL "" FORCE)
	set(ENABLE_EXCEPTIONS_ON_MSVC ON CACHE BOOL "" FORCE)
	set(SPIRV_TOOLS_BUILD_STATIC ON CACHE BOOL "" FORCE)
	set(SPIRV_SKIP_EXECUTABLES OFF CACHE BOOL "" FORCE)
	set(SPIRV_CHECK_CONTEXT ON CACHE BOOL "" FORCE)
	set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)

	# Include Sub-Project
	get_external_include_path(lib_inc_path "spirv-tools")
	get_external_source_path(lib_src_path "spirv-tools")

	FetchContent_Declare(
			libspirv-tools
			GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
			GIT_TAG v2024.1
			GIT_SHALLOW ON
			GIT_PROGRESS ON
			SOURCE_DIR "${lib_src_path}"
			LOG_DOWNLOAD ON
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			libspirv-tools
	)

	# Setup Library
	add_library(${PROJECT_NAME}::${static_alias} ALIAS SPIRV-Tools-static)
	add_library(${PROJECT_NAME}::${reduce_alias} ALIAS SPIRV-Tools-reduce)
	add_library(${PROJECT_NAME}::${opt_alias} ALIAS SPIRV-Tools-opt)
	add_library(${PROJECT_NAME}::${link_alias} ALIAS SPIRV-Tools-link)
	add_library(${PROJECT_NAME}::${lint_alias} ALIAS SPIRV-Tools-lint)
	add_library(${PROJECT_NAME}::${diff_alias} ALIAS SPIRV-Tools-diff)

	install_includes(
			DIRECTORY "${lib_src_path}/include/"
			DESTINATION "${lib_inc_path}/"
	)

	export(TARGETS SPIRV-Tools-static NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${static_alias}/${static_alias}-export.cmake)
	export(TARGETS SPIRV-Tools-reduce NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${reduce_alias}/${reduce_alias}-export.cmake)
	export(TARGETS SPIRV-Tools-opt NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${opt_alias}/${opt_alias}-export.cmake)
	export(TARGETS SPIRV-Tools-link NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${link_alias}/${link_alias}-export.cmake)
	export(TARGETS SPIRV-Tools-lint NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${lint_alias}/${lint_alias}-export.cmake)
	export(TARGETS SPIRV-Tools-diff NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${diff_alias}/${diff_alias}-export.cmake)

	# Deployment
	get_external_lib_path(lib_dist_path "spirv-tools")
	foreach (target
			SPIRV-Tools-static SPIRV-Tools-reduce SPIRV-Tools-opt SPIRV-Tools-link SPIRV-Tools-lint SPIRV-Tools-diff
			spirv-as spirv-dis spirv-link spirv-opt spirv-val spirv-reduce spirv-cfg spirv-diff spirv-lint spirv-objdump
	)
		set_batch_target_properties(${target} ${lib_dist_path})
	endforeach ()

endfunction()

# Push to setup
add_managed_library(NAME libspirv-tools CALL library_spirv_tools_init DEPS libspirv-headers)