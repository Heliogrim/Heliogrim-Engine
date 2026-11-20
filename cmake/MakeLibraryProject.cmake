include_guard(GLOBAL)

function(_prepare_link_libs libs out)
	list(TRANSFORM libs STRIP)
	set("${out}" ${libs} PARENT_SCOPE)
endfunction()

function(_apply_default_compile_settings target)
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

#
# Module Providers
#

# Folds a sub-project's headers and prefix into its root's module; `out_libraries` are the sub's libraries without the root.
function(_record_module_sub sub out_root out_libraries)
	get_property(declared GLOBAL PROPERTY global_module_declared)
	get_property(root GLOBAL PROPERTY global_module_sub_${sub}_root)
	if (NOT root IN_LIST declared)
		message(FATAL_ERROR "Sub-project `${sub}` is `SUB_OF` `${root}`, which is not declared by `make_library_project`.")
	endif ()

	get_property(headers GLOBAL PROPERTY global_module_sub_${sub}_headers)
	get_property(libraries GLOBAL PROPERTY global_module_sub_${sub}_libraries)
	list(REMOVE_ITEM libraries "${PROJECT_NAME}::${root}")

	set_property(GLOBAL APPEND PROPERTY global_module_${root}_headers ${headers})
	set_property(GLOBAL APPEND PROPERTY global_module_${root}_prefixes "${sub}")

	set(${out_root} "${root}" PARENT_SCOPE)
	set(${out_libraries} "${libraries}" PARENT_SCOPE)
endfunction()

function(_merge_module_subs)
	get_property(subs GLOBAL PROPERTY global_module_subs)

	foreach (sub IN LISTS subs)
		_record_module_sub("${sub}" root libraries)
		set_property(GLOBAL APPEND PROPERTY global_module_project_${root}_libraries ${libraries})

		get_src_path(proj_src_dir)
		name_to_target(root_target "${root}")
		get_property(sources GLOBAL PROPERTY global_module_sub_${sub}_sources)
		get_property(headers GLOBAL PROPERTY global_module_sub_${sub}_headers)
		get_property(sub_dir GLOBAL PROPERTY global_module_sub_${sub}_dir)

		if (sources)
			target_sources(${root_target} PRIVATE ${sources})
		endif ()

		if (headers)
			target_sources(${root_target}
					PRIVATE FILE_SET private_HEADERS TYPE HEADERS BASE_DIRS ${sub_dir} FILES ${headers}
					INTERFACE FILE_SET interface_HEADERS TYPE HEADERS BASE_DIRS ${proj_src_dir} FILES ${headers}
			)
		endif ()

		if (libraries)
			target_link_libraries(${root_target} PRIVATE ${libraries})
		endif ()

		add_library(${PROJECT_NAME}::${sub} ALIAS ${root_target})
	endforeach ()
endfunction()

function(_finalize_module_interfaces)
	get_src_path(proj_src_dir)
	_merge_module_subs()
	get_property(roots GLOBAL PROPERTY global_module_roots)

	foreach (root IN LISTS roots)
		get_property(headers GLOBAL PROPERTY global_module_${root}_headers)
		get_property(prefixes GLOBAL PROPERTY global_module_${root}_prefixes)
		get_property(exclude GLOBAL PROPERTY global_module_${root}_exclude)

		name_to_target(root_target "${root}")
		_make_module_provider(provider "${root}")

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

		get_property(known GLOBAL PROPERTY global_module_providers)
		set(import_libraries "")
		foreach (imported IN LISTS MODULE_INTERFACE_IMPORTS)
			if (imported IN_LIST known OR imported IN_LIST roots)
				list(APPEND import_libraries "${PROJECT_NAME}::${imported}.Module")
			else ()
				message(FATAL_ERROR "Headers of `${root}` import `${imported}`, which no module provider declares.")
			endif ()
		endforeach ()

		target_link_libraries(${provider} PRIVATE ${import_libraries})
	endforeach ()

	get_property(projects GLOBAL PROPERTY global_module_projects)
	foreach (project IN LISTS projects)
		get_property(targets GLOBAL PROPERTY global_module_project_${project}_targets)
		_module_closure("${project}" providers)
		foreach (target IN LISTS targets)
			target_link_libraries(${target} PRIVATE ${providers})
		endforeach ()
	endforeach ()
endfunction()

# Module providers `name` needs: its own module, the modules in its `LIBRARIES`, and theirs, transitively
#  (a dependency's headers import that dependency's modules). Vendor providers are listed as `<name>.Module`.
function(_module_closure name out)
	get_property(roots GLOBAL PROPERTY global_module_roots)
	set(pending "${name}")
	set(visited "")
	set(providers "")
	while (pending)
		list(POP_FRONT pending current)
		if (current IN_LIST visited)
			continue()
		endif ()
		list(APPEND visited "${current}")

		get_property(module GLOBAL PROPERTY global_module_sub_${current}_root)
		if (NOT module)
			set(module "${current}")
		endif ()
		if (module IN_LIST roots)
			list(APPEND providers "${PROJECT_NAME}::${module}.Module")
		endif ()

		get_property(libraries GLOBAL PROPERTY global_module_project_${current}_libraries)
		foreach (library IN LISTS libraries)
			if (NOT library MATCHES "^${PROJECT_NAME}::(.+)$")
				continue()
			endif ()
			set(dependency "${CMAKE_MATCH_1}")
			if (dependency MATCHES "\\.Module$")
				list(APPEND providers "${library}")
			else ()
				list(APPEND pending "${dependency}")
			endif ()
		endforeach ()
	endwhile ()

	list(REMOVE_DUPLICATES providers)
	set(${out} "${providers}" PARENT_SCOPE)
endfunction()

# Declares a project's `LIBRARIES` for the module closure; `ARGN` are the targets that link the providers it needs.
function(_record_module_project name libraries)
	set_property(GLOBAL APPEND PROPERTY global_module_projects "${name}")
	set_property(GLOBAL PROPERTY global_module_project_${name}_libraries ${libraries})
	set_property(GLOBAL PROPERTY global_module_project_${name}_targets ${ARGN})
	_defer_finalize_module_interfaces()
endfunction()

function(_defer_finalize_module_interfaces)
	get_property(finalize_deferred GLOBAL PROPERTY global_module_finalize_deferred)
	if (NOT finalize_deferred)
		cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL _finalize_module_interfaces)
		set_property(GLOBAL PROPERTY global_module_finalize_deferred ON)
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

	_defer_finalize_module_interfaces()
endfunction()

function(make_library_project)

	# MODULE: generate a named module `LIB_NAME` from the bundle's headers (`MODULE_EXCLUDE`: headers kept textual).
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
	if (args_MODULE)
		set_property(GLOBAL APPEND PROPERTY global_module_roots "${args_LIB_NAME}")
		set_property(GLOBAL APPEND PROPERTY global_module_${args_LIB_NAME}_headers ${header_files})
		set_property(GLOBAL APPEND PROPERTY global_module_${args_LIB_NAME}_prefixes "${args_LIB_NAME}")
		set_property(GLOBAL APPEND PROPERTY global_module_${args_LIB_NAME}_exclude ${args_MODULE_EXCLUDE})
	endif ()
	_record_module_project("${args_LIB_NAME}" "${args_LIBRARIES}" ${target})

	add_library(${PROJECT_NAME}::${args_LIB_NAME} ALIAS ${target})

	# Export Downstream
	export(TARGETS ${target} NAMESPACE ${PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

	# Project Options, Include Directories, Compile Definitions and Options
	_apply_default_compile_settings(${target})
	
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