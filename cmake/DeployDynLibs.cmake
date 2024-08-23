
set(OPTION_DEPLOY_ENABLED ON)

function(enable_deploy status)
	if (NOT ${status})
		set(OPTION_DEPLOY_ENABLED ${status} PARENT_SCOPE)
		message(STATUS "Deployment of dlls skipped: Manually disabled")

		return()
	endif ()
endfunction()

function(add_deploy_to_target target)

	if (NOT ${OPTION_DEPLOY_ENABLED})
		message(STATUS "Skipped deployment: Manually disabled")
		return()
	endif ()

	get_target_property(link_libs ${target} LINK_LIBRARIES)
	gen_target_name_id(name_id)

	list(LENGTH link_libs link_libs_length)
	set(next_length 0)
	while (NOT next_length EQUAL link_libs_length)
		list(LENGTH link_libs link_libs_length)
		foreach (level ${link_libs})
			if (TARGET ${level})
				get_target_property(tmp ${level} LINK_LIBRARIES)
				list(APPEND link_libs ${tmp})
				list(REMOVE_DUPLICATES link_libs)
			endif ()
		endforeach ()
		list(LENGTH link_libs next_length)
	endwhile ()

	set(__local_build_type "${CMAKE_BUILD_TYPE}")
	string(TOUPPER "${__local_build_type}" __local_build_type)

	set(deployable "")
	set(rename "")

	foreach (candidate ${link_libs})
		if (TARGET ${candidate})
			get_target_property(candidate_type ${candidate} TYPE)
			if (candidate_type STREQUAL "SHARED_LIBRARY")
				get_target_property(candidate_run_dll_path ${candidate} RUNTIME_OUTPUT_DIRECTORY)
				set(candidate_run_dll_name "")
				set(candidate_postfix)
				set(candidate_suffix)
				set(candidate_soversion)

				# Check primary name

				if (NOT "${candidate_run_dll_name}" AND DEFINED CMAKE_BUILD_TYPE)
					get_target_property(candidate_run_dll_name ${candidate} "LIBRARY_OUTPUT_NAME_${__local_build_type}")
				endif ()

				if ("${candidate_run_dll_name}" MATCHES "NOT")
					get_target_property(candidate_run_dll_name ${candidate} LIBRARY_OUTPUT_NAME)
				endif ()

				if ("${candidate_run_dll_name}" MATCHES "NOT" AND DEFINED CMAKE_BUILD_TYPE)
					get_target_property(candidate_run_dll_name ${candidate} "RUNTIME_OUTPUT_NAME_${__local_build_type}")
				endif ()

				if ("${candidate_run_dll_name}" MATCHES "NOT")
					get_target_property(candidate_run_dll_name ${candidate} RUNTIME_OUTPUT_NAME)
				endif ()

				if ("${candidate_run_dll_name}" MATCHES "NOT" AND DEFINED CMAKE_BUILD_TYPE)
					get_target_property(candidate_run_dll_name ${candidate} "${__local_build_type}_OUTPUT_NAME")
				endif ()

				if ("${candidate_run_dll_name}" MATCHES "NOT")
					get_target_property(candidate_run_dll_name ${candidate} "OUTPUT_NAME")
				endif ()

				# Check Postfix

				if (NOT "${candidate_postfix}")
					get_target_property(candidate_postfix ${candidate} "${__local_build_type}_POSTFIX")
				endif ()

				if ("${candidate_postfix}" MATCHES "NOT")
					get_target_property(candidate_postfix ${candidate} "POSTFIX")
				endif ()

				if ("${candidate_postfix}" MATCHES "NOT")
					set(candidate_postfix "")
				endif ()

				# Check custom suffix

				if (NOT "${candidate_suffix}")
					get_target_property(candidate_suffix ${candidate} "SUFFIX")
				endif ()

				if ("${candidate_suffix}" MATCHES "NOT")
					set(candidate_suffix "${CMAKE_SHARED_LIBRARY_SUFFIX}")
				endif ()

				# Check so version

				if (NOT "${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
					get_target_property(candidate_soversion ${candidate} "SOVERSION")
				endif ()

				if ("${candidate_soversion}" MATCHES "NOT")
					set(candidate_soversion "")
				endif ()

				if (DEFINED candidate_run_dll_path AND DEFINED candidate_run_dll_name)
					# file(GLOB_RECURSE deployable "${proj_lib_dir}/**/bin/**/$<CONFIG>**/${name_id}**/*.dll")

					set(__target_name "${CMAKE_SHARED_LIBRARY_PREFIX}${candidate_run_dll_name}${candidate_postfix}${candidate_suffix}")
					list(APPEND deployable "${candidate_run_dll_path}/${__target_name}")

					if (NOT "${candidate_soversion}" STREQUAL "")
						set(__target_name "${__target_name}.${candidate_soversion}")
					endif ()

					list(APPEND rename "${__target_name}")
				endif ()
			endif ()
		endif ()
	endforeach ()

	# Append Sanitizer DLLs
	if (SANITIZER_DEPLOY_DLLS)
		foreach (pattern IN LISTS SANITIZER_DEPLOY_DLLS)
			file(GLOB tmp "${pattern}")
			list(APPEND deployable ${tmp})
			list(APPEND rename ${tmp})
		endforeach ()
	endif ()


	list(REMOVE_DUPLICATES deployable)
	list(LENGTH deployable deployable_size)

	if (deployable_size LESS_EQUAL 0)
		message(STATUS "Exit dynamic lib deployment, cause there are none.")
		return()
	endif ()

	# Sort for renaming and stable copy

	set(commands "")
	get_shared_dist_path(proj_dist_dir)
	foreach (copy IN ZIP_LISTS deployable rename)
		if ("${copy_0}" STREQUAL "${copy_1}")
			list(APPEND commands "COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${copy_0}\" \"${proj_dist_dir}\"")
		else ()
			list(APPEND commands "COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${copy_0}\" \"${proj_dist_dir}/${copy_1}\"")
		endif ()
	endforeach ()
	list(JOIN commands " " commands)

	cmake_language(EVAL CODE "
		add_custom_command(
			TARGET ${target} POST_BUILD
			${commands}
		)
	")
endfunction()