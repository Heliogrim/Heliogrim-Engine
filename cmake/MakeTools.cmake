include_guard(GLOBAL)

#
# Module Providers
#

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

function(_finalize_module_interfaces)
	get_src_path(proj_src_dir)
	_merge_module_subs()
	get_property(roots GLOBAL PROPERTY global_module_roots)

	foreach (root IN LISTS roots)
		get_property(headers GLOBAL PROPERTY global_module_${root}_headers)
		get_property(prefixes GLOBAL PROPERTY global_module_${root}_prefixes)
		get_property(exclude GLOBAL PROPERTY global_module_${root}_exclude)

		name_to_target(root_target "${root}")
		make_module_provider(provider "${root}")

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

# Public Functions

# Provider target `${PROJECT_NAME}-<name>.Module` (alias `${PROJECT_NAME}::<name>.Module`), built like a library project.
function(make_module_provider out name)
	name_to_target(provider "${name}.Module")
	add_library(${provider} OBJECT)
	add_library(${PROJECT_NAME}::${name}.Module ALIAS ${provider})
	set_property(GLOBAL APPEND PROPERTY global_module_providers "${name}")

	# Bundles linking the provider are exported, so the provider has to be too.
	export(TARGETS ${provider} NAMESPACE ${PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${provider}/${provider}-export.cmake)
	apply_default_compile_settings(${provider})

	# External targets only: importers get their usage requirements (e.g. Jolt's `-mavx2`), and a BMI must match them,
	#  else CMake 4.4 builds a second, synthetic BMI per consumer.
	target_link_libraries(${provider} PRIVATE ${DEFAULT_LIBRARIES})
	set(${out} "${provider}" PARENT_SCOPE)
endfunction()

function(defer_finalize_module_interfaces)
	get_property(finalize_deferred GLOBAL PROPERTY global_module_finalize_deferred)
	if (NOT finalize_deferred)
		cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL _finalize_module_interfaces)
		set_property(GLOBAL PROPERTY global_module_finalize_deferred ON)
	endif ()
endfunction()

# Declares a project's `LIBRARIES` for the module closure; `ARGN` are the targets that link the providers it needs.
function(record_module_project name libraries)
	set_property(GLOBAL APPEND PROPERTY global_module_projects "${name}")
	set_property(GLOBAL PROPERTY global_module_project_${name}_libraries ${libraries})
	set_property(GLOBAL PROPERTY global_module_project_${name}_targets ${ARGN})
	defer_finalize_module_interfaces()
endfunction()

function(apply_default_compile_settings target)
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