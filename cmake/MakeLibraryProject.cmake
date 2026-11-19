include_guard(GLOBAL)

function(__prepare_link_libs libs out)
	list(TRANSFORM libs STRIP)
	set("${out}" ${libs} PARENT_SCOPE)
endfunction()

function(__apply_default_compile_settings target)
	get_src_path(proj_src_dir)

	# Project Options
	set_target_properties(${target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS})

	# Include Directories
	target_include_directories(${target} SYSTEM PRIVATE ${proj_src_dir})
	target_include_directories(${target} SYSTEM PUBLIC ${DEFAULT_INCLUDE_DIRECTORIES})

	# Compile
	target_compile_definitions(
			${target}
			PRIVATE
			${DEFAULT_COMPILE_DEFINITIONS}
	)

	# Compile Options
	target_compile_options(
			${target}
			PRIVATE
			${DEFAULT_COMPILE_OPTIONS}
	)
endfunction()

#
# Module Providers
#

function(_finalize_module_interfaces)
	get_src_path(proj_src_dir)
	get_property(roots GLOBAL PROPERTY global_module_roots)

	foreach (root IN LISTS roots)
		get_property(headers GLOBAL PROPERTY global_module_${root}_headers)
		get_property(prefixes GLOBAL PROPERTY global_module_${root}_prefixes)
		get_property(exclude GLOBAL PROPERTY global_module_${root}_exclude)

		name_to_target(root_target "${root}")
		name_to_target(provider "${root}.Module")

		add_library(${provider} OBJECT)
		add_library(${PROJECT_NAME}::${root}.Module ALIAS ${provider})
		# Bundles linking the provider are exported, so the provider has to be too.
		export(TARGETS ${provider} NAMESPACE ${PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${provider}/${provider}-export.cmake)
		__apply_default_compile_settings(${provider})

		get_target_property(folder ${root_target} FOLDER)
		if (folder)
			set_target_properties(${provider} PROPERTIES FOLDER "${folder}")
		endif ()

		make_module_interface(
				TARGET ${provider}
				MODULE_NAME "${root}"
				BASE_DIR "${proj_src_dir}"
				HEADERS ${headers}
				OWN_PREFIXES ${prefixes}
				EXCLUDE ${exclude}
		)

		set(import_libraries "")
		foreach (imported IN LISTS MODULE_INTERFACE_IMPORTS)
			if (imported IN_LIST DEFAULT_MODULES)
				list(APPEND import_libraries "${PROJECT_NAME}::${imported}.Module")
			elseif (NOT imported STREQUAL "vulkan")
				message(FATAL_ERROR "Headers of `${root}` import `${imported}`, which is not in `DEFAULT_MODULES`.")
			endif ()
		endforeach ()

		# External targets only, so the interface compiles with the include dirs and definitions of its importers.
		target_link_libraries(
				${provider}
				PRIVATE
				${DEFAULT_LIBRARIES}
				${import_libraries}
		)
	endforeach ()
endfunction()

function(make_library_project)

	set(__single_var_options LIB_NAME LIB_FOLDER SOURCE FORCE_TYPE_STATIC)
	set(__multi_var_options LIBRARIES MODULE_EXCLUDE)
	cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	if (NOT DEFINED args_LIB_NAME)
		message(FATAL_ERROR "Target `${args_LIB_NAME}` is required parameter name to generate setup.")
		return()
	endif ()

	message(STATUS "Setup library project for target `${args_LIB_NAME}`.")
	name_to_target(target "${args_LIB_NAME}")

	list(LENGTH args_LIBRARIES link_libraries_size)
	if (link_libraries_size GREATER 0)
		__prepare_link_libs(${args_LIBRARIES} args_LIBRARIES)
		list(LENGTH args_LIBRARIES link_libraries_size)
	endif ()
	message(STATUS "Using `${link_libraries_size}` libraries at linker.")

	# Discover Sources
	get_src_path(proj_src_dir)

	if (DEFINED args_SOURCE)
		set(source_directory "${proj_src_dir}/${args_SOURCE}")
	else ()
		set(source_directory "${proj_src_dir}/${args_LIB_NAME}")
	endif ()

	file(GLOB_RECURSE header_files CONFIGURE_DEPENDS ${source_directory}/*.hpp ${source_directory}/*.h ${source_directory}/*.hh)
	file(GLOB_RECURSE source_files CONFIGURE_DEPENDS ${source_directory}/*.cpp ${source_directory}/*.c ${source_directory}/*.cc)

	# Sources
	map_source_groups(BASE_PATH "${source_directory}" SOURCES ${source_files} ${header_files})

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

	# Module Provider (generated once all bundles are known)
	if (args_LIB_NAME IN_LIST DEFAULT_MODULES)
		set_property(GLOBAL APPEND PROPERTY global_module_roots "${args_LIB_NAME}")
		set_property(GLOBAL APPEND PROPERTY global_module_${args_LIB_NAME}_headers ${header_files})
		set_property(GLOBAL APPEND PROPERTY global_module_${args_LIB_NAME}_prefixes "${args_LIB_NAME}")
		set_property(GLOBAL APPEND PROPERTY global_module_${args_LIB_NAME}_exclude ${args_MODULE_EXCLUDE})

		get_property(finalize_deferred GLOBAL PROPERTY global_module_finalize_deferred)
		if (NOT finalize_deferred)
			cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL _finalize_module_interfaces)
			set_property(GLOBAL PROPERTY global_module_finalize_deferred ON)
		endif ()
	endif ()

	add_library(${PROJECT_NAME}::${args_LIB_NAME} ALIAS ${target})

	# Export Downstream
	export(TARGETS ${target} NAMESPACE ${PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

	# Project Options, Include Directories, Compile Definitions and Options
	__apply_default_compile_settings(${target})
	if (NOT DEFINED args_LIB_FOLDER)
		try_generate_project_groups("${args_LIB_NAME}" may_sub_folder)
		if (DEFINED may_sub_folder)
			set_target_properties(${target} PROPERTIES FOLDER "${IDE_FOLDER}/${may_sub_folder}")
		endif ()

	elseif (DEFINED args_LIB_FOLDER)
		set_target_properties(${target} PROPERTIES FOLDER "${args_LIB_FOLDER}")
	endif ()

	# Libraries
	set(module_libraries ${DEFAULT_MODULE_LIBRARIES})
	list(REMOVE_ITEM module_libraries "${PROJECT_NAME}::${args_LIB_NAME}.Module")
	target_link_libraries(
			${target}
			PRIVATE
			${args_LIBRARIES}
			${DEFAULT_LIBRARIES}
			${module_libraries}
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