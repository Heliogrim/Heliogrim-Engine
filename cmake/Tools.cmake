include_guard(GLOBAL)

# Internal Cached Variables

if ("${CMAKE_GENERATOR}" MATCHES "Visual Studio")
	set(_IS_VS_GENERATOR ON CACHE INTERNAL "" FORCE)
else ()
	set(_IS_VS_GENERATOR OFF CACHE INTERNAL "" FORCE)
endif ()

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
	# Warning: This is just a temporary fix; We need to propagate the target directory for binary files through the managed layer
	if (NOT PROJECT_IS_TOP_LEVEL)
		if ("${CMAKE_FOLDER}" STREQUAL "External" AND "${IDE_FOLDER}" STREQUAL "Heliogrim" AND PROJECT_SOURCE_DIR MATCHES "compile$")
			cmake_path(GET PROJECT_SOURCE_DIR PARENT_PATH __managed_parent)
			set(${out} "${__managed_parent}/bin")
		else ()
			set(${out} "${PROJECT_SOURCE_DIR}/dist")
		endif ()
	else ()
		set(${out} "${PROJECT_SOURCE_DIR}/dist")
	endif ()
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

# IDE Directories

function(map_source_groups)

	# Check whether generator target supports source groups
	if (NOT ${_IS_VS_GENERATOR})
		return()
	endif ()

	# Parse Arguments
	set(__single_var_options BASE_PATH)
	set(__multi_var_options SOURCES)
	cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	if (NOT DEFINED args_BASE_PATH OR NOT DEFINED args_SOURCES)
		message(FATAL_ERROR "Calling `map_source_groups` requires as `BASE_PATH` and corresponding `SOURCES`.")
	endif ()

	foreach (obj ${args_SOURCES})
		set(relative "${obj}")
		if (IS_ABSOLUTE "${obj}")
			file(RELATIVE_PATH relative "${args_BASE_PATH}" "${obj}")
		endif ()

		get_filename_component(mapping "${relative}" PATH)

		if ("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Windows" AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")

			set(prefix_msvc "Other")
			if ("${relative}" MATCHES "^.*\.(c|cc|cpp)$")
				set(prefix_msvc "Sources")
			elseif ("${relative}" MATCHES "^.*\.(h|hh|hpp)$")
				set(prefix_msvc "Headers")
			endif ()

			string(REPLACE "/" "\\" mapping_msvc "${prefix_msvc}/${mapping}")
			source_group("${mapping_msvc}" FILES "${obj}")
		else ()
		endif ()
	endforeach ()
endfunction()

function(try_generate_project_groups name may_out_dir)

	# Check for "." delimiter
	set(delimiter ".")
	string(STRIP "${name}" subject)
	string(FIND "${subject}" "${delimiter}" check_delimiter)

	# If failed -> Check for "-" delimiter
	if (${check_delimiter} EQUAL -1)
		set(delimiter "-")
		string(FIND "${subject}" "${delimiter}" check_delimiter)
	endif ()

	if (NOT ${check_delimiter} EQUAL -1)
		string(FIND "${subject}" "${delimiter}" last REVERSE)
		string(SUBSTRING "${subject}" 0 ${last} remaining)
		string(REPLACE "${delimiter}" "/" sub_folder "${remaining}")
		set(${may_out_dir} "${sub_folder}" PARENT_SCOPE)
	else ()
		unset(${may_out_dir} PARENT_SCOPE)
	endif ()
endfunction()

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