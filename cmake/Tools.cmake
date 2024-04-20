
macro(list_directories retval cwd)

	file(GLOB sub-dir LIST_DIRECTORIES true CONFIGURE_DEPENDS ${cwd}/*)
	set(list_of "")

	foreach (dir ${sub-dir})
		if (IS_DIRECTORY "${dir}")
			list(APPEND list_of "${dir}")
		endif ()
	endforeach ()

	set(${retval} ${list_of})
endmacro()

# Directories

macro(get_projects_path out)
	set(${out} "${PROJECT_SOURCE_DIR}/projects")
endmacro()

macro(get_src_path out)
	set(${out} "${PROJECT_SOURCE_DIR}/src")
endmacro()

macro(get_library_path out)
	set(${out} "${PROJECT_SOURCE_DIR}/library")
endmacro()

macro(get_dist_base_path out)
	set(${out} "${PROJECT_SOURCE_DIR}/dist")
endmacro()

macro(get_shared_dist_path out)
	get_dist_base_path(base_path)
	gen_target_name_id(name_id)
	set(${out} "${base_path}/$<CONFIG>/${name_id}")
endmacro()

macro(get_external_lib_path out name)
	get_library_path(base_path)
	gen_target_name_id(name_id)
	set(${out} "${base_path}/${name}/bin/$<CONFIG>/${name_id}")
endmacro()

macro(get_external_include_path out name)
	get_library_path(base_path)
	set(${out} "${base_path}/${name}/include")
endmacro()

macro(get_external_source_path out name)
	get_library_path(base_path)
	set(${out} "${base_path}/${name}/compile")
endmacro()

# Naming

macro(name_to_target out name)
	set(${out} "${PROJECT_NAME}-${name}")
endmacro()

macro(name_to_external_alias out name)
	set(${out} "External-${name}")
endmacro()

macro(gen_target_name_id out)
	string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" local_system_processor)
	string(TOLOWER "${CMAKE_SYSTEM_NAME}" local_system_name)
	string(TOLOWER "${CMAKE_HOST_SYSTEM_NAME}" local_host_system_name)
	string(TOLOWER "${CMAKE_CXX_COMPILER_ABI}" local_cxx_compiler_abi)
	string(TOLOWER "${CMAKE_C_COMPILER_ABI}" local_c_compiler_abi)

	#

	set(parts)

	if (NOT ${local_system_name} STREQUAL ${local_host_system_name})
		list(APPEND parts "cross")
	endif ()

	list(APPEND parts ${local_system_processor})

	if (NOT ${local_system_name} STREQUAL ${local_host_system_name})
		list(APPEND parts ${local_system_name})
		list(APPEND parts ${local_host_system_name})
	else ()
		list(APPEND parts ${local_system_name})
	endif ()

	if (NOT ${local_cxx_compiler_abi} STREQUAL "")
		list(APPEND parts ${local_cxx_compiler_abi})
	elseif (NOT ${local_c_compiler_abi} STREQUAL "")
		list(APPEND parts ${local_c_compiler_abi})
	endif ()

	list(JOIN parts "-" ${out})
endmacro()

# Target Properties

macro(set_batch_target_properties target destination)
	set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${destination})
	set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${destination})
	set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${destination})
	set_target_properties(${target} PROPERTIES PDB_OUTPUT_DIRECTORY ${destination})
endmacro()

function(install_includes)

	# Parse Arguments
	set(__single_var_options DESTINATION PATCH_ONLY TARGET TARGET_SOURCE EXCLUDE_PATTERN INCLUDE_REGEX)
	set(__multi_var_options GLOB SOURCE DIRECTORY)
	cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	# Verify Argument Configuration
	if (NOT DEFINED args_DESTINATION)
		message(FATAL_ERROR "Argument `DESTINATION` of type path is required when calling `install_includes`.")
	endif ()

	if (NOT DEFINED args_GLOB AND NOT DEFINED args_SOURCE AND NOT DEFINED args_DIRECTORY AND NOT DEFINED args_TARGET)
		message(FATAL_ERROR "Calling `install_includes` requires at least one source of `GLOB`, `SOURCE`, `DIRECTORY` or `TARGETS`.")
	endif ()

	if (DEFINED args_TARGET AND NOT DEFINED args_TARGET_SOURCE)
		message(FATAL_ERROR "Calling `install_includes` with `TARGET` argument requires `TARGET_SOURCE` in addition.")
	endif ()

	if (DEFINED args_INCLUDE_REGEX AND DEFINED args_EXCLUDE_PATTERN)
		message(WARNING "When calling `install_includes` `INCLUDE_REGEX` will take precedence over `EXCLUDE_PATTERN`.")
		unset(args_EXCLUDE_PATTERN)
	endif ()

	# Query files to install
	set(single_sources "")
	set(check_sources "")

	if (DEFINED args_SOURCE)
		foreach (obj ${args_SOURCE})
			list(APPEND single_sources "${obj}")
			get_filename_component(tmp "${obj}" NAME)
			list(APPEND check_sources "${tmp}")
		endforeach ()
	endif ()

	if (DEFINED args_GLOB)
		file(GLOB_RECURSE discovered "${args_GLOB}")
		foreach (obj ${discovered})
			# TODO: Transform ${obj} to relative path
			list(APPEND single_sources "${obj}")
			get_filename_component(tmp "${obj}" NAME)
			list(APPEND check_sources "${tmp}")
		endforeach ()
	endif ()

	if (DEFINED args_TARGET AND DEFINED args_TARGET_SOURCE)
		get_target_property(public ${args_TARGET} PUBLIC_HEADER)
		foreach (candidate ${public})
			file(REAL_PATH ${candidate} obj BASE_DIRECTORY "${args_TARGET_SOURCE}")
			list(APPEND single_sources "${obj}")
			get_filename_component(tmp "${obj}" NAME)
			list(APPEND check_sources "${tmp}")
		endforeach ()
	endif ()

	# Process already present files, removed obsolete and patch
	if (NOT DEFINED args_PATCH_ONLY)

		if (DEFINED args_DIRECTORY)
			foreach (directory "${args_DIRECTORY}")
				file(GLOB_RECURSE discovered "${directory}*.*")
				foreach (obj ${discovered})
					file(RELATIVE_PATH tmp "${directory}" ${obj})
					list(APPEND check_sources "${tmp}")
				endforeach ()
			endforeach ()
		endif ()

		# Check already present files
		set(present "")
		if (EXISTS "${args_DESTINATION}")
			file(GLOB_RECURSE retrieved "${args_DESTINATION}/*.*")
			foreach (obj ${retrieved})
				file(RELATIVE_PATH tmp "${args_DESTINATION}" ${obj})
				list(APPEND present "${tmp}")
			endforeach ()
		endif ()

		# Sort sets to guarantee hash equality
		list(SORT check_sources COMPARE FILE_BASENAME CASE SENSITIVE ORDER ASCENDING)
		list(SORT present COMPARE FILE_BASENAME CASE SENSITIVE ORDER ASCENDING)

		string(SHA256 source_hash "${check_sources}")
		string(SHA256 present_hash "${present}")

		message(DEBUG "Checking hashes: ${source_hash} <-> ${present_hash}")

		# If we found a delta, erase obsolete entries
		if (NOT ${source_hash} EQUAL ${present_hash})
			list(REMOVE_ITEM present ${check_sources})
			message(DEBUG "Difference Set: ${present}")

			macro(_local_rm_recurse base dir)

				get_filename_component(tmp ${dir} DIRECTORY)
				if ("${tmp}" STREQUAL "${base}")
					return()
				endif ()

				file(GLOB check_any "${tmp}/*")
				list(LENGTH check_any check_any_length)

				if (${check_any_length} LESS_EQUAL 0)
					file(REMOVE_RECURSE "${tmp}")
					_local_rm_recurse(${base} ${tmp})
				endif ()

			endmacro()

			foreach (obj ${present})
				file(REAL_PATH "${obj}" tmp BASE_DIRECTORY "${args_DESTINATION}")
				file(REMOVE "${tmp}")
				_local_rm_recurse("${args_DESTINATION}" "${tmp}")
			endforeach ()
		endif ()

	endif ()

	# Patch / install files to destination
	if (DEFINED args_DIRECTORY)
		if (DEFINED args_INCLUDE_REGEX)
			file(COPY ${args_DIRECTORY} DESTINATION "${args_DESTINATION}" FILES_MATCHING REGEX "${args_INCLUDE_REGEX}")
		elseif (DEFINED args_EXCLUDE_PATTERN)
			file(COPY ${args_DIRECTORY} DESTINATION "${args_DESTINATION}" PATTERN "${args_EXCLUDE_PATTERN}" EXCLUDE)
		else ()
			file(COPY ${args_DIRECTORY} DESTINATION "${args_DESTINATION}")
		endif ()
	endif ()

	string(LENGTH "${single_sources}" single_sources_length)
	if (${single_sources_length} GREATER 0)
		if (DEFINED args_INCLUDE_REGEX)
			file(COPY ${single_sources} DESTINATION "${args_DESTINATION}" FILES_MATCHING REGEX "${args_INCLUDE_REGEX}")
		elseif (DEFINED args_EXCLUDE_PATTERN)
			file(COPY ${single_sources} DESTINATION "${args_DESTINATION}" PATTERN "${args_EXCLUDE_PATTERN}" EXCLUDE)
		else ()
			file(COPY ${single_sources} DESTINATION "${args_DESTINATION}")
		endif ()
	endif ()

endfunction()