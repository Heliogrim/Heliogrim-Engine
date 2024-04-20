function(library_glslang_init)

	cmake_minimum_required(VERSION 3.27.7)
	include(FetchContent)

	name_to_external_alias(glslang_alias "glslang")
	name_to_external_alias(generic_code_gen_alias "GenericCodeGen")
	name_to_external_alias(machine_indep_alias "MachineIndependent")
	name_to_external_alias(os_dep_alias "OSDependent")
	name_to_external_alias(default_limits_alias "glslang-default-resource-limits")
	name_to_external_alias(spirv_alias "SPIRV")
	name_to_external_alias(spv_remapper_alias "SPVRemapper")

	#
	set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
	set(BUILD_EXTERNAL OFF CACHE INTERNAL "" FORCE)
	set(GLSLANG_TESTS OFF CACHE INTERNAL "" FORCE)
	set(GLSLANG_TESTS_DEFAULT OFF CACHE INTERNAL "" FORCE)
	set(GLSLANG_ENABLE_INSTALL OFF CACHE INTERNAL "" FORCE)
	set(ENABLE_GLSLANG_JS OFF CACHE INTERNAL "" FORCE)

	set(ENABLE_SPVREMAPPER ON CACHE INTERNAL "" FORCE)
	set(ENABLE_GLSLANG_BINARIES OFF CACHE INTERNAL "" FORCE)

	set(ENABLE_HLSL ON CACHE INTERNAL "" FORCE)
	set(ENABLE_RTTI OFF CACHE INTERNAL "" FORCE)
	set(ENABLE_EXCEPTIONS ON CACHE INTERNAL "" FORCE)

	set(ENABLE_OPT ON CACHE INTERNAL "" FORCE)
	set(ALLOW_EXTERNAL_SPIRV_TOOLS OFF CACHE INTERNAL "" FORCE)

	# Include Sub-Project
	get_external_include_path(lib_inc_path "glslang")
	get_external_source_path(lib_src_path "glslang")

	FetchContent_Declare(
			libglslang
			GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
			GIT_TAG 14.1.0
			GIT_SHALLOW ON
			GIT_PROGRESS ON
			SOURCE_DIR "${lib_src_path}"
	)
	FetchContent_MakeAvailable(
			libglslang
	)

	# Setup Library
	add_library(${PROJECT_NAME}::${glslang_alias} ALIAS glslang)
	add_library(${PROJECT_NAME}::${generic_code_gen_alias} ALIAS GenericCodeGen)
	add_library(${PROJECT_NAME}::${machine_indep_alias} ALIAS MachineIndependent)
	add_library(${PROJECT_NAME}::${os_dep_alias} ALIAS OSDependent)
	add_library(${PROJECT_NAME}::${default_limits_alias} ALIAS glslang-default-resource-limits)
	add_library(${PROJECT_NAME}::${spirv_alias} ALIAS SPIRV)
	add_library(${PROJECT_NAME}::${spv_remapper_alias} ALIAS SPVRemapper)

	install_includes(DIRECTORY "${lib_src_path}/glslang/HLSL/" DESTINATION "${lib_inc_path}/glslang/HLSL/")
	install_includes(DIRECTORY "${lib_src_path}/glslang/Include/" DESTINATION "${lib_inc_path}/glslang/Include/")
	install_includes(DIRECTORY "${lib_src_path}/glslang/MachineIndependent/" DESTINATION "${lib_inc_path}/glslang/MachineIndependent/")
	install_includes(DIRECTORY "${lib_src_path}/glslang/Public/" DESTINATION "${lib_inc_path}/glslang/Public/")
	install_includes(DIRECTORY "${lib_src_path}/SPIRV/" DESTINATION "${lib_inc_path}/glslang/SPIRV/")

	export(TARGETS glslang NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${glslang_alias}/${glslang_alias}-export.cmake)
	export(TARGETS GenericCodeGen NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${generic_code_gen_alias}/${generic_code_gen_alias}-export.cmake)
	export(TARGETS MachineIndependent NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${machine_indep_alias}/${machine_indep_alias}-export.cmake)
	export(TARGETS OSDependent NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${os_dep_alias}/${os_dep_alias}-export.cmake)
	export(TARGETS glslang-default-resource-limits NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${default_limits_alias}/${default_limits_alias}-export.cmake)
	export(TARGETS SPIRV NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${spirv_alias}/${spirv_alias}-export.cmake)
	export(TARGETS SPVRemapper NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${spv_remapper_alias}/${spv_remapper_alias}-export.cmake)

	# Deployment
	get_external_lib_path(lib_dist_path "glslang")
	foreach (target glslang GenericCodeGen MachineIndependent OSDependent glslang-default-resource-limits SPIRV SPVRemapper)
		set_batch_target_properties(${target} ${lib_dist_path})
	endforeach ()

endfunction()

# Push to setup
add_managed_library(NAME libglslang CALL library_glslang_init DEPS libspirv-tools libspirv-headers)