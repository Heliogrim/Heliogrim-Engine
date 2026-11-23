include_guard(GLOBAL)

include(${CMAKE_CURRENT_LIST_DIR}/MakeTools.cmake)

function(_prepare_link_libs libs out)
	list(TRANSFORM libs STRIP)
	set("${out}" ${libs} PARENT_SCOPE)
endfunction()

# Source directory (`<src>/<source>`, else `<src>/<name>`) with its headers and sources, grouped for IDEs.
function(_discover_sources name source out_dir out_headers out_sources)
	get_src_path(proj_src_dir)

	if (source)
		set(source_directory "${proj_src_dir}/${source}")
	else ()
		set(source_directory "${proj_src_dir}/${name}")
	endif ()

	file(GLOB_RECURSE header_files CONFIGURE_DEPENDS ${source_directory}/*.hpp ${source_directory}/*.h ${source_directory}/*.hh)
	file(GLOB_RECURSE source_files CONFIGURE_DEPENDS ${source_directory}/*.cpp ${source_directory}/*.c ${source_directory}/*.cc)
	map_source_groups(BASE_PATH "${source_directory}" SOURCES ${source_files} ${header_files})

	set(${out_dir} "${source_directory}" PARENT_SCOPE)
	set(${out_headers} ${header_files} PARENT_SCOPE)
	set(${out_sources} ${source_files} PARENT_SCOPE)
endfunction()

# Module kind of a bundle: `HANDWRITTEN` (a `CxxModule.ixx` at its root), `GENERATED` (`MODULE`), else empty.
function(_module_kind source_directory generate out)
	if (EXISTS "${source_directory}/CxxModule.ixx")
		if (generate)
			message(FATAL_ERROR "`${source_directory}` has a hand-written `CxxModule.ixx`; drop `MODULE`, it would generate the same module again.")
		endif ()
		set(${out} HANDWRITTEN PARENT_SCOPE)
	elseif (generate)
		set(${out} GENERATED PARENT_SCOPE)
	else ()
		set(${out} "" PARENT_SCOPE)
	endif ()
endfunction()

function(make_library_sub_project)

	set(__single_var_options LIB_NAME SUB_OF)
	set(__multi_var_options LIBRARIES)
	cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	if (NOT DEFINED args_LIB_NAME OR NOT DEFINED args_SUB_OF)
		message(FATAL_ERROR "Sub-project requires `LIB_NAME` and `SUB_OF`, got `${args_LIB_NAME}` of `${args_SUB_OF}`.")
		return()
	endif ()

	message(STATUS "Setup library sub-project `${args_LIB_NAME}` of `${args_SUB_OF}`.")

	list(LENGTH args_LIBRARIES link_libraries_size)
	if (link_libraries_size GREATER 0)
		_prepare_link_libs(${args_LIBRARIES} args_LIBRARIES)
	endif ()

	_discover_sources("${args_LIB_NAME}" "" source_directory header_files source_files)

	set_property(GLOBAL APPEND PROPERTY global_module_subs "${args_LIB_NAME}")
	set_property(GLOBAL PROPERTY global_module_sub_${args_LIB_NAME}_root "${args_SUB_OF}")
	set_property(GLOBAL PROPERTY global_module_sub_${args_LIB_NAME}_dir "${source_directory}")
	set_property(GLOBAL PROPERTY global_module_sub_${args_LIB_NAME}_sources ${source_files})
	set_property(GLOBAL PROPERTY global_module_sub_${args_LIB_NAME}_headers ${header_files})
	set_property(GLOBAL PROPERTY global_module_sub_${args_LIB_NAME}_libraries ${args_LIBRARIES})
	set_property(GLOBAL PROPERTY global_module_project_${args_LIB_NAME}_libraries ${args_LIBRARIES})

	defer_finalize_module_interfaces()
endfunction()

function(make_library_project)

	# MODULE: generate a named module `LIB_NAME` from the bundle's headers (`MODULE_EXCLUDE`: headers kept textual).
	#  A bundle with a hand-written `CxxModule.ixx` omits it.
	set(__single_var_options LIB_NAME LIB_FOLDER SOURCE FORCE_TYPE_STATIC)
	set(__multi_var_options LIBRARIES MODULE_EXCLUDE)
	cmake_parse_arguments(args "MODULE" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	if (NOT DEFINED args_LIB_NAME)
		message(FATAL_ERROR "Target `${args_LIB_NAME}` is required parameter name to generate setup.")
		return()
	endif ()

	message(STATUS "Setup library project for target `${args_LIB_NAME}`.")
	name_to_target(target "${args_LIB_NAME}")

	list(LENGTH args_LIBRARIES link_libraries_size)
	if (link_libraries_size GREATER 0)
		_prepare_link_libs(${args_LIBRARIES} args_LIBRARIES)
		list(LENGTH args_LIBRARIES link_libraries_size)
	endif ()
	message(STATUS "Using `${link_libraries_size}` libraries at linker.")

	# Discover Sources
	get_src_path(proj_src_dir)
	_discover_sources("${args_LIB_NAME}" "${args_SOURCE}" source_directory header_files source_files)

	# Library
	if (DEFINED args_FORCE_TYPE_STATIC)
		add_library(${target} ${source_files})
		target_sources(${target}
				PRIVATE FILE_SET private_HEADERS TYPE HEADERS FILES ${header_files}
				INTERFACE FILE_SET interface_HEADERS TYPE HEADERS BASE_DIRS ${proj_src_dir} FILES ${header_files}
		)
	else ()
		add_library(${target} OBJECT ${source_files})
		target_sources(${target}
				PRIVATE FILE_SET private_HEADERS TYPE HEADERS FILES ${header_files}
				INTERFACE FILE_SET interface_HEADERS TYPE HEADERS BASE_DIRS ${proj_src_dir} FILES ${header_files}
		)
	endif ()

	if (DEFINED CMAKE_CXX_SCAN_FOR_MODULES AND CMAKE_CXX_SCAN_FOR_MODULES)
		file(GLOB_RECURSE module_interface_files CONFIGURE_DEPENDS ${source_directory}/*.ixx)
		file(GLOB_RECURSE module_source_files CONFIGURE_DEPENDS ${source_directory}/*.mxx ${source_directory}/*.ccm ${source_directory}/*.cxxm ${source_directory}/*.cppm)

		# Even an empty CXX_MODULES file set makes consumers expect a collated module map, which header-only bundles never produce.
		if (module_source_files OR module_interface_files)
			target_sources(${target} PUBLIC FILE_SET CXX_MODULES FILES ${module_source_files} ${module_interface_files})
		endif ()
	endif ()

	# Module Provider and Sub-Projects (generated once all bundles are known)
	set_property(GLOBAL APPEND PROPERTY global_module_declared "${args_LIB_NAME}")
	_module_kind("${source_directory}" "${args_MODULE}" module_kind)
	if (module_kind STREQUAL "HANDWRITTEN")
		# The bundle is its own provider: generated interfaces importing it link `<name>.Module`.
		add_library(${PROJECT_NAME}::${args_LIB_NAME}.Module ALIAS ${target})
		set_property(GLOBAL APPEND PROPERTY global_module_providers "${args_LIB_NAME}")
	elseif (module_kind STREQUAL "GENERATED")
		set_property(GLOBAL APPEND PROPERTY global_module_roots "${args_LIB_NAME}")
		set_property(GLOBAL APPEND PROPERTY global_module_${args_LIB_NAME}_headers ${header_files})
		set_property(GLOBAL APPEND PROPERTY global_module_${args_LIB_NAME}_prefixes "${args_LIB_NAME}")
		set_property(GLOBAL APPEND PROPERTY global_module_${args_LIB_NAME}_exclude ${args_MODULE_EXCLUDE})
	endif ()
	record_module_project("${args_LIB_NAME}" "${args_LIBRARIES}" ${target})

	add_library(${PROJECT_NAME}::${args_LIB_NAME} ALIAS ${target})

	# Export Downstream
	export(TARGETS ${target} NAMESPACE ${PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

	# Project Options, Include Directories, Compile Definitions and Options
	apply_default_compile_settings(${target})

	if (NOT DEFINED args_LIB_FOLDER)
		try_generate_project_groups("${args_LIB_NAME}" may_sub_folder)
		if (DEFINED may_sub_folder)
			set_target_properties(${target} PROPERTIES FOLDER "${IDE_FOLDER}/${may_sub_folder}")
		endif ()

	elseif (DEFINED args_LIB_FOLDER)
		set_target_properties(${target} PROPERTIES FOLDER "${args_LIB_FOLDER}")
	endif ()

	# Libraries (module providers beyond the defaults, the own one included, follow in `_finalize_module_interfaces`)
	target_link_libraries(
			${target}
			PRIVATE
			${args_LIBRARIES}
			${DEFAULT_LIBRARIES}
			${DEFAULT_MODULE_LIBRARIES}
	)

	# Linker Options
	target_link_options(
			${target}
			PRIVATE
			${DEFAULT_LINKER_OPTIONS}
	)
	set_target_lto(${target})

	# Target Health
	# Deployment
	get_shared_dist_path(proj_dist_path)
	set_batch_target_properties(${target} ${proj_dist_path})
endfunction()